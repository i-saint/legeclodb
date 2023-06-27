#pragma once
#include "ldb.h"
#include "task_feeder.h"

namespace ldb::lookup {

class Options
{
public:
    struct EffectParam {
        bool enabled_ = false;
        int valueType_ = 0;
        float limit_ = 0;
        float weight_ = 1.0f;
    };
    struct PrioritizeParam
    {
        int item_ = 0;
        int owner_ = 0;
    };

    int maxUnits_ = 5;
    int maxActive_ = 2;
    uint32_t allowOnBattle_ : 1 { true };
    uint32_t allowProbability_ : 1 { true };
    uint32_t allowSingleUnitBuff_ : 1 { false };
    uint32_t allowSymbolSkill_ : 1 { false };
    uint32_t allowSupportActive_ : 1 { true };
    uint32_t allowMaxStackValue_ : 1 { true };
    uint32_t classFilter_ = ~0;
    uint32_t symbolFilter_ = ~0;
    uint32_t rarityFilter_ = ~0;
    std::vector<EffectParam> targets_;
    std::vector<PrioritizeParam> excluded_;
    std::vector<PrioritizeParam> prioritized;

    void setup(val data);
};

class SerarchState
{
public:
    SerarchState* parent_{};
    std::array<int, 64> usedSlots_{};
    std::array<int, 64> totalAmounts_{};
    std::bitset<2048> usedEntities_{};
};

class ResultHolder
{
public:
    bool operator<(const ResultHolder& r) const;
    val toJs() const;
    val toJsTree() const;

    // root -> current
    void each(const std::function<void(ResultHolder&)>& cb) {
        if (parent_)
            parent_->each(cb);
        cb(*this);
    }
    void each(const std::function<void(const ResultHolder&)>& cb) const {
        if (parent_)
            const_cast<const ResultHolder*>(parent_)->each(cb);
        cb(*this);
    }

public:
    ResultHolder* parent_{};

    SupportCharacter* supChr_{};
    ist::fixed_vector<Skill*, 3> supSkills_{};

    ist::fixed_vector<Item*, 4> items_{};

    MainCharacter* mainChr_{};
    ist::fixed_vector<Skill*, 4> mainSkills_{};

    MainCharacter* summonChr_{};
    ist::fixed_vector<Skill*, 4> summonSkills_{};

    ist::fixed_vector<SkillEffect*, 32> usedEffects_{};
    ist::fixed_vector<SkillEffect*, 8> deniedEffects_{};

    int unitCount_ = 0;     // 
    int skillCount_ = 0;    // 
    float scoreTotal_ = 0;  // 親階層含む全合計

    float scoreMain_ = 0;    // 
    float scoreSupport_ = 0; // 
    float scoreItems_ = 0;   // 現在の階層のみのスコア
};


class LookupContext;

class SearchContext
{
public:
    SearchContext();
    void setup(LookupContext* ctx, val data);
    void beginSearch();

    void wait();
    bool isComplete() const;
    int getSearchCount() const;
    val getResult() const;
    void test();

public:
    void searchRecursive(SerarchState *pstate, ResultHolder* pr, int depth = 0);
    bool submitResult(const ResultHolder& result);

    bool effectCondition(const SkillEffect& effect) const;
    bool skillCondition(const Skill& skill) const;
    float getEffectValue(const SkillEffect& effect) const;

    float getScoreEst(const SerarchState& state, const Skill& skill, const Entity& owner) const;
    float getScoreEst(const SerarchState& state, const MainCharacter& obj) const;
    float getScoreEst(const SerarchState& state, const SupportCharacter& obj) const;
    float getScoreEst(const SerarchState& state, const Item& obj) const;

    float getScore(ResultHolder& dst, SerarchState& state, const MainCharacter& obj);
    float getScore(ResultHolder& dst, SerarchState& state, const SupportCharacter& obj);
    float getScore(ResultHolder& dst, SerarchState& state, const Item& obj);
    void updateState(SerarchState& state, const ResultHolder& result);

public:
    const LookupContext* lctx_{};
    Options opt_{};

    std::vector<const MainCharacter*> mainChrs_;
    std::vector<const SupportCharacter*> supChrs_;
    std::vector<const Item*> weapons_, armors_, helmets_, accessories_;

    ResultHolder* bestResult_{};
    ist::fixed_vector<ResultHolder, 10> bestTree_;
    std::atomic_int searchCount_{};
    TaskFeeder taskFeeder_;
};

class LookupContext : public BaseContext
{
public:
    LookupContext(val data);
    val beginSearch(val option);
    void test(val v);

private:
};

} // namespace ldb::lookup

