#ifndef CARD_MATCH_GAME_CARD_VIEW_H
#define CARD_MATCH_GAME_CARD_VIEW_H

#include <functional>

#include "cocos2d.h"
#include "models/CardModel.h"

namespace cardgame {

/**
 * 单张卡牌视图。
 * 负责卡牌正反面展示、点击事件接收以及基础移动动画。
 */
class CardView : public cocos2d::Node {
public:
    /**
     * 创建一张卡牌视图。
     * @param cardModel 初始化使用的卡牌模型。
     * @return 创建好的卡牌视图对象。
     */
    static CardView* create(const CardModel& cardModel);

    /**
     * 使用卡牌模型初始化视图。
     * @param cardModel 初始化使用的卡牌模型。
     * @return 初始化是否成功。
     */
    bool initWithModel(const CardModel& cardModel);

    /**
     * 根据模型刷新视图表现。
     * @param cardModel 最新卡牌模型。
     * @param showFaceUp 是否显示正面。
     * @param isInteractive 当前是否允许点击。
     */
    void refresh(const CardModel& cardModel, bool showFaceUp, bool isInteractive);

    /**
     * 设置当前卡牌是否允许点击。
     * @param isInteractive 是否允许点击。
     */
    void setInteractive(bool isInteractive);

    /**
     * 设置卡牌缩放值。
     * @param cardScale 目标缩放。
     */
    void setCardScale(float cardScale);

    /**
     * 设置点击回调。
     * @param tapCallback 点击后回调的函数对象。
     */
    void setTapCallback(const std::function<void(int)>& tapCallback);

    /**
     * 播放移动动画。
     * @param targetPosition 目标左下角坐标。
     * @param targetScale 目标缩放。
     * @param duration 动画时长。
     * @param onComplete 动画完成后的回调。
     */
    void playMoveTo(
        const cocos2d::Vec2& targetPosition,
        float targetScale,
        float duration,
        const std::function<void()>& onComplete);

    /** 播放翻开后的轻量强调动画。 */
    void playRevealAnimation();

    /** 返回当前卡牌 ID。 */
    int getCardId() const;

private:
    void rebuildFrontFace(const CardModel& cardModel);
    bool isTouchInside(const cocos2d::Touch* touch) const;

    int _cardId;
    bool _isInteractive;
    float _cardScale;
    cocos2d::Sprite* _shadowSprite;
    cocos2d::Sprite* _highlightSprite;
    cocos2d::Node* _frontRoot;
    cocos2d::Node* _backRoot;
    std::function<void(int)> _tapCallback;
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_CARD_VIEW_H
