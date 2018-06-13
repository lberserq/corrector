#include "spell_checker_impl.h"


#include <algorithm>
#include <codecvt>
#include <exception>
#include <fstream>
#include <iterator>
#include <locale>

#include <iostream>

namespace spell_checker
{

namespace
{
    class WorldListReaderImpl : public IWordListReader
    {
        wordlist_t GetWordList(const path_t &inDicPath) override
        {
            std::cerr << inDicPath.generic_string().c_str() << "\n";
            std::ifstream fin(inDicPath.generic_string().c_str(), std::ios_base::binary | std::ios_base::in);
            fin.seekg(0, std::ios_base::end);
            auto length = fin.tellg();
            fin.seekg(0, std::ios_base::beg);
            std::string rawData(static_cast<std::size_t>(length), '\0'); // read raw data in UTF-8
            fin.read(&rawData[0], length);
            string_t data = std::wstring_convert<
                    std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(rawData);

            constexpr string_t::value_type delim = u'-';
            std::transform(data.begin(), data.end(), data.begin(), [=](char16_t inChar) {
                //some generic hacks, Ideal impl should be done with locale
                static const string_t ff = u"1234567890|-+/*=!?%.,;:~^@$&#_()[]{}<>«»'\" \t\r\n";
                if (std::find(ff.begin(), ff.end(), inChar) == ff.end()) {
                    return inChar;
                }
                return delim;
            });


            wordlist_t res;
            for (string_t::size_type i = 0; i != string_t::npos;)
            {
                const string_t::size_type firstNonFilt = data.find_first_not_of(delim, i + 1);
                if (firstNonFilt == string_t::npos)
                  break;
                auto end = data.find(delim, firstNonFilt);
                res.push_back(data.substr(firstNonFilt, end - firstNonFilt));

                i = end;
            }
            return res;
        }
    };


    size_t dlevidist(const string_t &inA, const string_t &inB)
    {
        std::vector<std::vector<size_t>> mat(inA.size() + 1);
        for(auto &row : mat) {
            row.resize(inB.size() + 1);
        }
        for (size_t i = 0; i < inA.size() + 1; ++i) {
            for (size_t j = 0; j < inB.size() + 1; ++j) {
                if (std::min(i, j) == 0) {
                    mat[i][j] = std::max(i, j);
                } else if (i > 1 && j > 1 && inA[i] == inB[j - 1] && inB[j] == inA[i - 1]) {
                    bool cond = inA[i - 1] != inB[j - 1];
                    mat[i][j] = std::min(std::min(mat[i - 1][j] + 1, mat[i][j - 1] + 1), std::min(mat[i - 2][j - 2] + 1, mat[i - 1][j - 1] + cond));
                } else {
                    mat[i][j] = std::min(std::min(mat[i - 1][j] + 1, mat[i][j - 1] + 1), mat[i - 1][j - 1] + (inA[i - 1] != inB[j - 1]));
                }
            }
        }
        return mat.back().back();
    }

    string_t getMinDiff(const string_t &inWord, edits_with_value_t &&edits)
    {
        corrected_string_t res{string_t(), inWord.size()};
        for (auto &suggestion : edits)  {
            if (suggestion.second < res.second) {
                res = suggestion;
            }
        }
        if (res.second == 0) {
            return inWord;
        }
        return res.first;
    }

string_t toLowerStr(const string_t &inStr, const std::locale &inLocale)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> codecvt ;
    string8_t p = codecvt.to_bytes(inStr);
    std::transform(p.begin(), p.end(), p.begin(), [inLocale](char ch){return std::tolower(ch, inLocale);});
    return codecvt.from_bytes(p);
}

}



OneLangCorrector::OneLangCorrector(const std::locale &inLocale, const wordlist_t &inAlphabet, const wordlist_t &inWords)
    : m_locale(inLocale)
    , m_alphabet(inAlphabet)
{
    for (auto &word : inWords) {
        m_wordlist[toLowerStr(word, m_locale)]++;
    }
}


corrected_string_t OneLangCorrector::Correct(const string_t &inWordOrig) const
{
    string_t inWord = toLowerStr(inWordOrig, m_locale);
    if (m_wordlist.count(inWord) == 1) {
        return {inWord, 0u};
    }

    edits_t edits = GenerateEdits(inWord);
    dict_t suggestions = FilterKnownWords(edits);
    if (suggestions.size() > 0) {
         auto res =  std::max_element(suggestions.begin(), suggestions.end(), [](const corrected_string_t &i1, const corrected_string_t &i2){return i1.second < i2.second;})->first;
         return {res, dlevidist(inWord, res)};
    }
    for (auto &edit : edits)
    {
        edits_t newEdits = GenerateEdits(edit);
        FilterKnownWords(newEdits, suggestions);
    }
    if (suggestions.size() > 0) {
        auto res = std::max_element(suggestions.begin(), suggestions.end(), [](const corrected_string_t &i1, const corrected_string_t &i2){return i1.second < i2.second;})->first;
        return {res, dlevidist(inWord, res)};
    }
    return {string_t(), suggestions.size()};
}

edits_t OneLangCorrector::GenerateEdits(const string_t &inWord) const
{
    edits_t result;
    for(size_t i = 0; i < inWord.size(); ++i) { //delete one
        result.push_back(inWord.substr(0, i) + inWord.substr(i + 1));
    }

    for(size_t i = 0; i < inWord.size() - 1; ++i) { //transpose
        result.push_back(inWord.substr(0, i) + inWord[i + 1] + inWord[i] + inWord.substr(i + 2));
    }

    for (auto &alpha : m_alphabet) {
        for(size_t i = 0; i < inWord.size(); ++i) { //change letter
            result.push_back(inWord.substr(0, i) + alpha + inWord.substr(i + 1));
        }

        for(size_t i = 0; i < inWord.size() + 1; ++i) { //insert
            result.push_back(inWord.substr(0, i) + alpha + inWord.substr(i));
        }
    }
    return result;
}

dict_t OneLangCorrector::FilterKnownWords(const edits_t &inEdits) const
{
    dict_t val;
    FilterKnownWords(inEdits, val);
    return val;
}

void OneLangCorrector::FilterKnownWords(const edits_t &inEdits, dict_t &inOutDict) const
{
    for (auto &elem : inEdits)  {
        auto iter = m_wordlist.find(elem);
        if (iter != m_wordlist.end()) {
            inOutDict[iter->first] = iter->second;
        }
    }
}



SpellCheckerImpl::SpellCheckerImpl() : m_reader(std::make_shared<WorldListReaderImpl>())
{}

string_t SpellCheckerImpl::Correct(const string_t &inWord) const
{
    string_t procLower;
    locker_t locker(m_mtx);
    edits_with_value_t suggestions;
    for (auto &corrPair : m_correctors) {
        suggestions.push_back(corrPair.second->Correct(inWord));
    }
    return getMinDiff(inWord, std::move(suggestions));
}

objptr_t<ICorrectorConfigurator> SpellCheckerImpl::GetConfigurator()
{
    return shared_from_this();
}

void SpellCheckerImpl::SetReader(objptr_t<IWordListReader> inReader)
{
    if (!inReader) {
        throw std::logic_error("inReader must be non-null");
    }
    locker_t locker(m_mtx);
    m_reader = inReader;
}

void SpellCheckerImpl::AddLanguage(const string8_t &inLangCode, const wordlist_t &inAlphabet, const path_t &inDicPath)
{
    locker_t locker(m_mtx);
    if (m_correctors.count(inLangCode) > 0) {
        throw std::logic_error("Already has Corrector for this lang-code");
    }
    auto wordList = m_reader->GetWordList(inDicPath);
    m_correctors[inLangCode] = std::make_unique<OneLangCorrector>(std::locale(inLangCode), inAlphabet, std::move(wordList));
}

objptr_t<ICorrector> CreateCorrector()
{
    return std::make_shared<SpellCheckerImpl>();
}

}//spell_checker
