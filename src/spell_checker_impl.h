#ifndef src_spell_checker_impl_h__
#define src_spell_checker_impl_h_
#include "iface/spell_checker.h"

#include <locale>
#include <mutex>
#include <unordered_map>


namespace spell_checker
{
using corrected_string_t = std::pair<string_t, size_t>;
using edits_t = wordlist_t;
using edits_with_value_t = std::vector<corrected_string_t>;

//Norvig spell-checker
//http://norvig.com/spell-correct.html
class OneLangCorrector
{
public:
    explicit OneLangCorrector(const std::locale &inLocale, const wordlist_t &inAlphabet, const wordlist_t &inWords);
    corrected_string_t Correct(const string_t &inWord) const;
private:
    edits_t GenerateEdits(const string_t &inWord) const;
    dict_t FilterKnownWords(const edits_t &inEdits) const;
    void FilterKnownWords(const edits_t &inEdits, dict_t &inOutDict) const;



    std::locale m_locale;
    wordlist_t m_alphabet;
    dict_t m_wordlist;
};

class SpellCheckerImpl : public ICorrector, public ICorrectorConfigurator, public std::enable_shared_from_this<SpellCheckerImpl>
{
public:
    SpellCheckerImpl();
private:
    string_t Correct(const string_t &inWord) const;
    objptr_t<ICorrectorConfigurator> GetConfigurator();
    void SetReader(objptr_t<IWordListReader> inReader);
    void AddLanguage(const string8_t &inLangCode, const wordlist_t &inAlphabet, const path_t &inDicPath);

    objptr_t<IWordListReader> m_reader;
    mutable std::mutex m_mtx;
    using locker_t = std::lock_guard<std::mutex>;

    std::unordered_map<string8_t, std::unique_ptr<OneLangCorrector>> m_correctors;
};

}

#endif //src_id_generator_impl_h__
