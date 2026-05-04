#ifndef CARD_MATCH_GAME_GAME_VIEW_H
#define CARD_MATCH_GAME_GAME_VIEW_H

#include <functional>
#include <memory>
#include <unordered_map>

#include "cocos2d.h"
#include "controllers/GameController.h"
#include "models/GameModel.h"

namespace cardgame {

class CardView;

/**
 * 整局游戏主视图。
 * 负责场景背景、主牌区、备用牌堆、底牌历史区以及回退按钮的展示。
 */
class GameView : public cocos2d::Scene {
public:
    /** 创建场景入口。 */
    static cocos2d::Scene* createScene();

    ~GameView() override;

    /** 初始化游戏主视图。 */
    bool init() override;

    /**
     * 注册主牌区卡牌点击回调。
     * @param callback 用户点击主牌区卡牌后的回调。
     */
    void setOnPlayfieldCardClick(const std::function<void(int)>& callback);

    /**
     * 注册备用牌点击回调。
     * @param callback 用户点击备用牌后的回调。
     */
    void setOnDrawPileClick(const std::function<void()>& callback);

    /**
     * 注册回退按钮点击回调。
     * @param callback 用户点击回退按钮后的回调。
     */
    void setOnUndoClick(const std::function<void()>& callback);

    /** 注册重新开局点击回调。 */
    void setOnRestartClick(const std::function<void()>& callback);

    /**
     * 首次展示整局游戏数据。
     * @param gameModel 初始游戏模型。
     */
    void presentGame(const GameModel& gameModel);

    /**
     * 将最新模型同步到视图。
     * @param gameModel 最新游戏模型。
     * @param excludedCardId 需要在同步时暂时跳过的卡牌 ID；默认不过滤。
     */
    void syncGame(const GameModel& gameModel, int excludedCardId = -1);

    /**
     * 查询某张卡牌在指定模型下的目标展示信息。
     * @param gameModel 用于计算展示布局的游戏模型。
     * @param cardId 需要查询的卡牌 ID。
     * @param outPosition 输出的左下角坐标。
     * @param outScale 输出的缩放。
     * @param outFaceUp 输出的正反面状态。
     * @param outInteractive 输出的点击开关。
     * @param outZOrder 输出的层级。
     * @return 是否成功计算出展示结果。
     */
    bool getCardPresentation(
        const GameModel& gameModel,
        int cardId,
        cocos2d::Vec2* outPosition,
        float* outScale,
        bool* outFaceUp,
        bool* outInteractive,
        int* outZOrder) const;

    /**
     * 播放卡牌移动到当前底牌位的动画。
     * @param cardId 需要移动的卡牌 ID。
     * @param onComplete 动画完成后的回调。
     */
    void playCardMoveToTray(int cardId, const std::function<void()>& onComplete);

    /**
     * 将指定卡牌播放回某个目标模型下的展示位置。
     * @param cardId 需要移动的卡牌 ID。
     * @param targetModel 目标游戏模型。
     * @param onComplete 动画完成后的回调。
     */
    void playCardMoveToPresentation(
        int cardId,
        const GameModel& targetModel,
        const std::function<void()>& onComplete);

    /** 播放一组新翻开卡牌的强调动画。 */
    void playRevealAnimations(const std::vector<int>& cardIds);

    /**
     * 设置整局游戏的交互开关。
     * @param isEnabled 是否允许用户输入。
     */
    void setInteractionEnabled(bool isEnabled);

    /**
     * 设置回退按钮是否可用。
     * @param isEnabled 是否可触发回退。
     */
    void setUndoEnabled(bool isEnabled);

    /** 隐藏结算弹窗。 */
    void hideResultOverlay();

    /** 展示胜负结算弹窗。 */
    void showResultOverlay(bool isVictory, int starCount, int remainingDrawCards);

    /** 返回当前底牌位的左下角锚点。 */
    cocos2d::Vec2 getTrayAnchor() const;

    CREATE_FUNC(GameView);

private:
    void createStaticUi();
    void createResultOverlay();
    void bindTapToNode(cocos2d::Node* targetNode, const std::function<void()>& onTap);
    CardView* ensureCardView(const CardModel& cardModel);
    void updateDrawPileDecorations(const GameModel& gameModel);
    void updateUndoButtonVisual();
    void playEntranceAnimation(const GameModel& gameModel);
    int getRemainingPlayfieldCount(const GameModel& gameModel) const;

    std::unique_ptr<GameController> _gameController;
    cocos2d::Node* _cardLayer;
    cocos2d::Node* _topHudPanel;
    cocos2d::Node* _bottomPanel;
    cocos2d::LayerColor* _drawPileTapProxy;
    cocos2d::LayerColor* _undoButton;
    cocos2d::Label* _drawCountLabel;
    cocos2d::Label* _undoLabel;
    cocos2d::Label* _progressLabel;
    cocos2d::Label* _statusLabel;
    cocos2d::Node* _resultOverlay;
    cocos2d::Label* _resultTitleLabel;
    cocos2d::Label* _resultBodyLabel;
    cocos2d::Label* _resultStarsLabel;
    std::unordered_map<int, CardView*> _cardViews;
    std::function<void(int)> _onPlayfieldCardClick;
    std::function<void()> _onDrawPileClick;
    std::function<void()> _onUndoClick;
    std::function<void()> _onRestartClick;
    bool _isInteractionEnabled;
    bool _isUndoEnabled;
    bool _isResultVisible;
};

}  // namespace cardgame

#endif  // CARD_MATCH_GAME_GAME_VIEW_H
