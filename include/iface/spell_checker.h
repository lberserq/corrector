#ifndef iface_spell_checker_h__
#define iface_spell_checker_h__

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
namespace spell_checker
{
using string_t = std::u16string;
using string8_t = std::string;
template<class T>
using objptr_t = std::shared_ptr<T>;
using path_t = boost::filesystem::path;
using dict_t = std::unordered_map<string_t, size_t>;


struct ICorrectorConfigurator;
struct ICorrector : boost::noncopyable
{
    virtual ~ICorrector() = default;
	///<summary>Correct words from input string</summary>
    ///<param name="inWord">Word which would be corrected</param>
    ///<returns> Corrected string</returns>
    virtual string_t Correct(const string_t &inWord) const = 0;

    ///<summary>Gets corrector configurator, if avalible</summary>
    ///<returns> Configurator for this corrector</returns>
    virtual objptr_t<ICorrectorConfigurator> GetConfigurator() = 0;
};

using wordlist_t = std::vector<string_t>;

struct IWordListReader : boost::noncopyable
{
    virtual ~IWordListReader() = default;

    ///<summary>Reads WordList from specified path</summary>
    ///<param name="inDicPath">path to some string file</param>
    ///<returns>vector of string with words from source</returns>
    virtual wordlist_t GetWordList(const path_t &inDicPath) = 0;
};

struct ICorrectorConfigurator : boost::noncopyable
{
    virtual ~ICorrectorConfigurator() = default;

    ///<summary>Sets specified reader as current dict reader</summary>
    ///<param name="inReader">new reader object, should be non-null</param>
    virtual void SetReader(objptr_t<IWordListReader> inReader) = 0;

    ///<summary>AddsNewLanguage to corrector</summary>
    ///<param name="inLangCode">Language code</param>
    ///<param name="inAlphabet">Language alphabet</param>
    ///<param name="inDicPath">path to dictionary file</param>
    virtual void AddLanguage(const string8_t &inLangCode, const wordlist_t &inAlphabet, const path_t &inDicPath) = 0;
};

objptr_t<ICorrector> CreateCorrector();

}//spell_checker
#endif //iface_spell_checker_h
