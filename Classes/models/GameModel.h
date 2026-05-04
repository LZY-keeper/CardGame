#ifndef CARD_MATCH_GAME_GAME_MODEL_H
#define CARD_MATCH_GAME_GAME_MODEL_H

#include <string>
#include <vector>

#include "json/document.h"
#include "json/stringbuffer.h"
#include "json/writer.h"
#include "models/CardModel.h"

namespace cardgame {

/**
 * 整局游戏的运行时数据模型。
 * 负责存储所有卡牌、备用牌顺序、底牌历史以及翻牌状态刷新逻辑。
 */
class GameModel {
public:
    /** 所有运行时卡牌模型。 */
    std::vector<CardModel> cards;
    /** 备用牌堆卡牌 ID，从前到后表示抽取顺序。 */
    std::vector<int> drawPileCardIds;
    /** 底牌历史卡牌 ID，末尾元素为当前底牌。 */
    std::vector<int> trayHistoryCardIds;

    /**
     * 根据卡牌 ID 获取可写模型对象。
     * @param cardId 卡牌唯一 ID。
     * @return 对应模型指针；不存在时返回空指针。
     */
    CardModel* getCardById(int cardId) {
        for (size_t index = 0; index < cards.size(); ++index) {
            if (cards[index].cardId == cardId) {
                return &cards[index];
            }
        }
        return nullptr;
    }

    /**
     * 根据卡牌 ID 获取只读模型对象。
     * @param cardId 卡牌唯一 ID。
     * @return 对应模型指针；不存在时返回空指针。
     */
    const CardModel* getCardById(int cardId) const {
        for (size_t index = 0; index < cards.size(); ++index) {
            if (cards[index].cardId == cardId) {
                return &cards[index];
            }
        }
        return nullptr;
    }

    /** 返回当前底牌 ID；不存在时返回 -1。 */
    int getCurrentTrayCardId() const {
        return trayHistoryCardIds.empty() ? -1 : trayHistoryCardIds.back();
    }

    /** 返回备用牌堆顶部卡牌 ID；不存在时返回 -1。 */
    int getTopDrawPileCardId() const {
        return drawPileCardIds.empty() ? -1 : drawPileCardIds.front();
    }

    /** 根据当前存活的上层卡牌刷新主牌区翻开状态。 */
    void refreshPlayfieldFaceUpState() {
        for (size_t index = 0; index < cards.size(); ++index) {
            CardModel& cardModel = cards[index];
            if (!cardModel.isPlayfieldCard()) {
                continue;
            }

            bool isCovered = false;
            for (size_t coveredIndex = 0; coveredIndex < cardModel.coveredByCardIds.size(); ++coveredIndex) {
                const CardModel* coveringCard = getCardById(cardModel.coveredByCardIds[coveredIndex]);
                if (coveringCard != nullptr && coveringCard->isPlayfieldCard()) {
                    isCovered = true;
                    break;
                }
            }

            cardModel.isFaceUp = !isCovered;
        }
    }

    /**
     * 将当前游戏模型序列化为 JSON 字符串。
     * @return 序列化结果。
     */
    std::string toJsonString() const {
        rapidjson::Document document;
        document.SetObject();
        rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

        rapidjson::Value cardArray(rapidjson::kArrayType);
        for (size_t index = 0; index < cards.size(); ++index) {
            rapidjson::Value cardValue = cards[index].toJsonValue(allocator);
            cardArray.PushBack(cardValue, allocator);
        }

        rapidjson::Value drawPileArray(rapidjson::kArrayType);
        for (size_t index = 0; index < drawPileCardIds.size(); ++index) {
            drawPileArray.PushBack(drawPileCardIds[index], allocator);
        }

        rapidjson::Value trayHistoryArray(rapidjson::kArrayType);
        for (size_t index = 0; index < trayHistoryCardIds.size(); ++index) {
            trayHistoryArray.PushBack(trayHistoryCardIds[index], allocator);
        }

        document.AddMember("cards", cardArray, allocator);
        document.AddMember("drawPileCardIds", drawPileArray, allocator);
        document.AddMember("trayHistoryCardIds", trayHistoryArray, allocator);

        rapidjson::StringBuffer stringBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(stringBuffer);
        document.Accept(writer);
        return stringBuffer.GetString();
    }

    /**
     * 通过 JSON 字符串恢复游戏模型。
     * @param jsonText 序列化后的字符串。
     * @return 反序列化是否成功。
     */
    bool fromJsonString(const std::string& jsonText) {
        rapidjson::Document document;
        document.Parse<0>(jsonText.c_str());
        if (document.HasParseError() || !document.IsObject()) {
            return false;
        }
        if (!document.HasMember("cards") || !document["cards"].IsArray() ||
            !document.HasMember("drawPileCardIds") || !document["drawPileCardIds"].IsArray() ||
            !document.HasMember("trayHistoryCardIds") || !document["trayHistoryCardIds"].IsArray()) {
            return false;
        }

        cards.clear();
        drawPileCardIds.clear();
        trayHistoryCardIds.clear();

        const rapidjson::Value& cardArray = document["cards"];
        for (rapidjson::SizeType index = 0; index < cardArray.Size(); ++index) {
            cards.push_back(CardModel::fromJsonValue(cardArray[index]));
        }

        const rapidjson::Value& drawPileArray = document["drawPileCardIds"];
        for (rapidjson::SizeType index = 0; index < drawPileArray.Size(); ++index) {
            if (drawPileArray[index].IsInt()) {
                drawPileCardIds.push_back(drawPileArray[index].GetInt());
            }
        }

        const rapidjson::Value& trayHistoryArray = document["trayHistoryCardIds"];
        for (rapidjson::SizeType index = 0; index < trayHistoryArray.Size(); ++index) {
            if (trayHistoryArray[index].IsInt()) {
                trayHistoryCardIds.push_back(trayHistoryArray[index].GetInt());
            }
        }

        refreshPlayfieldFaceUpState();
        return true;
    }
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_GAME_MODEL_H
