#ifndef CARD_MATCH_GAME_CARD_MODEL_H
#define CARD_MATCH_GAME_CARD_MODEL_H

#include <vector>

#include "cocos2d.h"
#include "json/document.h"
#include "models/CardTypes.h"

namespace cardgame {

/**
 * 单张卡牌的运行时数据模型。
 * 负责存储卡牌点数、花色、区域归属、显示位置与遮挡关系。
 */
class CardModel {
public:
    /** 卡牌唯一 ID。 */
    int cardId;
    /** 卡牌点数。 */
    CardFaceType cardFace;
    /** 卡牌花色。 */
    CardSuitType cardSuit;
    /** 运行时所在区域。 */
    CardAreaType areaType;
    /** 当前逻辑位置，使用左下角坐标。 */
    cocos2d::Vec2 position;
    /** 初始布局位置，撤销与存档恢复时可复用。 */
    cocos2d::Vec2 initialPosition;
    /** 当前是否为翻开状态。 */
    bool isFaceUp;
    /** 盖住当前卡牌的上层卡牌 ID 列表。 */
    std::vector<int> coveredByCardIds;

    CardModel()
        : cardId(-1)
        , cardFace(CFT_NONE)
        , cardSuit(CST_NONE)
        , areaType(CAT_NONE)
        , position(cocos2d::Vec2::ZERO)
        , initialPosition(cocos2d::Vec2::ZERO)
        , isFaceUp(false) {
    }

    /** 返回当前是否位于主牌区。 */
    bool isPlayfieldCard() const {
        return areaType == CAT_PLAYFIELD;
    }

    /** 返回当前是否位于备用牌堆。 */
    bool isDrawPileCard() const {
        return areaType == CAT_DRAW_PILE;
    }

    /** 返回当前是否位于底牌历史区。 */
    bool isTrayCard() const {
        return areaType == CAT_TRAY;
    }

    /**
     * 序列化当前卡牌模型。
     * @param allocator RapidJSON 内存分配器。
     * @return 序列化结果。
     */
    rapidjson::Value toJsonValue(rapidjson::Document::AllocatorType& allocator) const {
        rapidjson::Value value(rapidjson::kObjectType);
        value.AddMember("cardId", cardId, allocator);
        value.AddMember("cardFace", static_cast<int>(cardFace), allocator);
        value.AddMember("cardSuit", static_cast<int>(cardSuit), allocator);
        value.AddMember("areaType", static_cast<int>(areaType), allocator);
        value.AddMember("isFaceUp", isFaceUp, allocator);

        rapidjson::Value positionValue(rapidjson::kObjectType);
        positionValue.AddMember("x", position.x, allocator);
        positionValue.AddMember("y", position.y, allocator);
        value.AddMember("position", positionValue, allocator);

        rapidjson::Value initialPositionValue(rapidjson::kObjectType);
        initialPositionValue.AddMember("x", initialPosition.x, allocator);
        initialPositionValue.AddMember("y", initialPosition.y, allocator);
        value.AddMember("initialPosition", initialPositionValue, allocator);

        rapidjson::Value coveredByValue(rapidjson::kArrayType);
        for (size_t index = 0; index < coveredByCardIds.size(); ++index) {
            coveredByValue.PushBack(coveredByCardIds[index], allocator);
        }
        value.AddMember("coveredByCardIds", coveredByValue, allocator);
        return value;
    }

    /**
     * 反序列化卡牌模型。
     * @param value JSON 对象。
     * @return 反序列化后的模型。
     */
    static CardModel fromJsonValue(const rapidjson::Value& value) {
        CardModel cardModel;
        if (!value.IsObject()) {
            return cardModel;
        }

        if (value.HasMember("cardId") && value["cardId"].IsInt()) {
            cardModel.cardId = value["cardId"].GetInt();
        }
        if (value.HasMember("cardFace") && value["cardFace"].IsInt()) {
            cardModel.cardFace = static_cast<CardFaceType>(value["cardFace"].GetInt());
        }
        if (value.HasMember("cardSuit") && value["cardSuit"].IsInt()) {
            cardModel.cardSuit = static_cast<CardSuitType>(value["cardSuit"].GetInt());
        }
        if (value.HasMember("areaType") && value["areaType"].IsInt()) {
            cardModel.areaType = static_cast<CardAreaType>(value["areaType"].GetInt());
        }
        if (value.HasMember("isFaceUp") && value["isFaceUp"].IsBool()) {
            cardModel.isFaceUp = value["isFaceUp"].GetBool();
        }
        if (value.HasMember("position") && value["position"].IsObject()) {
            const rapidjson::Value& positionValue = value["position"];
            cardModel.position = cocos2d::Vec2(
                positionValue["x"].GetFloat(),
                positionValue["y"].GetFloat());
        }
        if (value.HasMember("initialPosition") && value["initialPosition"].IsObject()) {
            const rapidjson::Value& initialPositionValue = value["initialPosition"];
            cardModel.initialPosition = cocos2d::Vec2(
                initialPositionValue["x"].GetFloat(),
                initialPositionValue["y"].GetFloat());
        }
        if (value.HasMember("coveredByCardIds") && value["coveredByCardIds"].IsArray()) {
            const rapidjson::Value& coveredByValue = value["coveredByCardIds"];
            for (rapidjson::SizeType index = 0; index < coveredByValue.Size(); ++index) {
                if (coveredByValue[index].IsInt()) {
                    cardModel.coveredByCardIds.push_back(coveredByValue[index].GetInt());
                }
            }
        }

        return cardModel;
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_CARD_MODEL_H
