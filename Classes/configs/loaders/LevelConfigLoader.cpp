#include "configs/loaders/LevelConfigLoader.h"

#include "base/CCDirector.h"
#include "json/document.h"
#include "json/error/en.h"
#include "platform/CCFileUtils.h"

namespace cardgame {

namespace {

bool parseCardArray(
    const rapidjson::Value& cardArray,
    std::vector<LevelCardConfig>* outCards) {
    if (outCards == nullptr || !cardArray.IsArray()) {
        return false;
    }

    outCards->clear();
    for (rapidjson::SizeType index = 0; index < cardArray.Size(); ++index) {
        const rapidjson::Value& cardValue = cardArray[index];
        if (!cardValue.IsObject()) {
            return false;
        }

        if (!cardValue.HasMember("CardFace") || !cardValue["CardFace"].IsInt() ||
            !cardValue.HasMember("CardSuit") || !cardValue["CardSuit"].IsInt() ||
            !cardValue.HasMember("Position") || !cardValue["Position"].IsObject()) {
            return false;
        }

        const rapidjson::Value& positionValue = cardValue["Position"];
        if (!positionValue.HasMember("x") || !positionValue["x"].IsNumber() ||
            !positionValue.HasMember("y") || !positionValue["y"].IsNumber()) {
            return false;
        }

        LevelCardConfig cardConfig;
        cardConfig.cardFace = static_cast<CardFaceType>(cardValue["CardFace"].GetInt());
        cardConfig.cardSuit = static_cast<CardSuitType>(cardValue["CardSuit"].GetInt());
        cardConfig.position = cocos2d::Vec2(
            positionValue["x"].GetFloat(),
            positionValue["y"].GetFloat());
        outCards->push_back(cardConfig);
    }

    return true;
}

}  // namespace

bool LevelConfigLoader::loadLevelConfig(const std::string& filePath, LevelConfig* outLevelConfig) {
    if (outLevelConfig == nullptr) {
        return false;
    }

    const std::string fileContent = cocos2d::FileUtils::getInstance()->getStringFromFile(filePath);
    if (fileContent.empty()) {
        cocos2d::log("LevelConfigLoader: file is empty or missing: %s", filePath.c_str());
        return false;
    }

    rapidjson::Document document;
    document.Parse<0>(fileContent.c_str());
    if (document.HasParseError()) {
        cocos2d::log(
            "LevelConfigLoader: parse error for %s, reason: %s",
            filePath.c_str(),
            rapidjson::GetParseError_En(document.GetParseError()));
        return false;
    }

    if (!document.IsObject() || !document.HasMember("Playfield") || !document.HasMember("Stack")) {
        cocos2d::log("LevelConfigLoader: invalid root fields in %s", filePath.c_str());
        return false;
    }

    LevelConfig levelConfig;
    if (!parseCardArray(document["Playfield"], &levelConfig.playfieldCards) ||
        !parseCardArray(document["Stack"], &levelConfig.stackCards) ||
        !levelConfig.isValid()) {
        cocos2d::log("LevelConfigLoader: invalid card array content in %s", filePath.c_str());
        return false;
    }

    *outLevelConfig = levelConfig;
    return true;
}

}  // namespace cardgame
