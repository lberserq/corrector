#include <gtest/gtest.h>
#include "iface/spell_checker.h"

using namespace spell_checker;
class CorrectorFixture : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_corrector = CreateCorrector();
    }
    ICorrector & GetCorrector()
    {
        return *m_corrector;
    }
private:
    objptr_t<ICorrector> m_corrector;

};

class EnTestFixture : public CorrectorFixture
{
public:
    void SetUp() override
    {
        CorrectorFixture::SetUp();
        wordlist_t engAlphabet;
        for(char16_t i = u'a'; i <= u'z'; ++i) {

            engAlphabet.push_back(string_t{i});
        }
        GetCorrector().GetConfigurator()->AddLanguage("en_US.UTF-8", engAlphabet, "en.txt");
    }
};


class EnRuTestFixture : public EnTestFixture
{
public:
    void SetUp() override
    {
        EnTestFixture::SetUp();
        wordlist_t ruAlphabet;
        for(char16_t i = u'а'; i <= u'я'; ++i) {
            ruAlphabet.push_back(string_t{i});
        }
        GetCorrector().GetConfigurator()->AddLanguage("ru_RU.UTF-8", ruAlphabet, "ru.txt");
    }

};

TEST_F(EnTestFixture, CorrectorDoesnotCorrectWordsFromDictionary)
{
    auto &corrector = GetCorrector();

    EXPECT_EQ(u"Dog", corrector.Correct(u"Dog"));
}

TEST_F(EnTestFixture, CorrectorCorrectWord)
{
    auto &corrector = GetCorrector();

    EXPECT_EQ(u"all", corrector.Correct(u"wll"));
}


TEST_F(EnRuTestFixture, CorrectorCorrectWord)
{
    auto &corrector = GetCorrector();

    EXPECT_EQ(u"да", corrector.Correct(u"bда"));
}


TEST_F(EnTestFixture, CorrectorProhibitAddExistingLanguage)
{
    auto &corrector = GetCorrector();

    EXPECT_THROW(corrector.GetConfigurator()->AddLanguage("en_US.UTF-8", {}, "en.txt"), std::logic_error);
}

TEST_F(CorrectorFixture, CorrectorProhibitSetEmptyReader)
{
    auto &corrector = GetCorrector();

    EXPECT_THROW(corrector.GetConfigurator()->SetReader(nullptr), std::logic_error);
}

