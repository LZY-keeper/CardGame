#include "views/CardView.h"

#include <new>

#include "2d/CCActionEase.h"
#include "2d/CCActionInstant.h"
#include "2d/CCLabel.h"
#include "2d/CCSprite.h"
#include "2d/CCActionInterval.h"
#include "base/CCEventDispatcher.h"
#include "base/CCEventListenerTouch.h"
#include "configs/models/CardResConfig.h"

namespace cardgame {

CardView* CardView::create(const CardModel& cardModel) {
    CardView* cardView = new (std::nothrow) CardView();
    if (cardView != nullptr && cardView->initWithModel(cardModel)) {
        cardView->autorelease();
        return cardView;
    }

    delete cardView;
    return nullptr;
}

bool CardView::initWithModel(const CardModel& cardModel) {
    if (!Node::init()) {
        return false;
    }

    _cardId = cardModel.cardId;
    _isInteractive = false;
    _cardScale = 1.0f;
    _shadowSprite = nullptr;
    _highlightSprite = nullptr;
    setCascadeOpacityEnabled(true);
    setContentSize(CardResConfig::getCardSourceSize());

    _shadowSprite = cocos2d::Sprite::create(CardResConfig::getCardBasePath());
    _shadowSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _shadowSprite->setPosition(cocos2d::Vec2(getContentSize().width * 0.5f, getContentSize().height * 0.5f - 10.0f));
    _shadowSprite->setColor(cocos2d::Color3B::BLACK);
    _shadowSprite->setOpacity(70);
    _shadowSprite->setScale(1.03f);
    addChild(_shadowSprite, 0);

    _highlightSprite = cocos2d::Sprite::create(CardResConfig::getCardBasePath());
    _highlightSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    _highlightSprite->setPosition(cocos2d::Vec2(getContentSize().width * 0.5f, getContentSize().height * 0.5f));
    _highlightSprite->setColor(cocos2d::Color3B(255, 234, 145));
    _highlightSprite->setOpacity(0);
    _highlightSprite->setScale(1.09f);
    addChild(_highlightSprite, 1);

    _frontRoot = Node::create();
    _backRoot = Node::create();
    addChild(_frontRoot, 2);
    addChild(_backRoot, 2);

    cocos2d::Sprite* backBaseSprite = cocos2d::Sprite::create(CardResConfig::getCardBasePath());
    backBaseSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    backBaseSprite->setPosition(cocos2d::Vec2(getContentSize().width * 0.5f, getContentSize().height * 0.5f));
    backBaseSprite->setColor(cocos2d::Color3B(54, 127, 231));
    _backRoot->addChild(backBaseSprite);

    cocos2d::Sprite* backSuitSprite = cocos2d::Sprite::create(CardResConfig::getSuitPath(CST_SPADES));
    if (backSuitSprite != nullptr) {
        backSuitSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        backSuitSprite->setPosition(cocos2d::Vec2(getContentSize().width * 0.5f, getContentSize().height * 0.5f + 18.0f));
        backSuitSprite->setColor(cocos2d::Color3B(255, 255, 255));
        backSuitSprite->setOpacity(84);
        backSuitSprite->setScale(2.6f);
        _backRoot->addChild(backSuitSprite);
    }

    cocos2d::Label* backLabel = cocos2d::Label::createWithSystemFont("DRAW", "Arial", 26);
    backLabel->setTextColor(cocos2d::Color4B(255, 255, 255, 220));
    backLabel->setPosition(cocos2d::Vec2(getContentSize().width * 0.5f, 50.0f));
    _backRoot->addChild(backLabel);

    rebuildFrontFace(cardModel);

    cocos2d::EventListenerTouchOneByOne* listener = cocos2d::EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(false);
    listener->onTouchBegan = [this](cocos2d::Touch* touch, cocos2d::Event*) {
        return _isInteractive && isTouchInside(touch);
    };
    listener->onTouchEnded = [this](cocos2d::Touch* touch, cocos2d::Event*) {
        if (_isInteractive && isTouchInside(touch) && _tapCallback) {
            _tapCallback(_cardId);
        }
    };
    _eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);

    return true;
}

void CardView::refresh(const CardModel& cardModel, bool showFaceUp, bool isInteractive) {
    _cardId = cardModel.cardId;
    _isInteractive = isInteractive;
    rebuildFrontFace(cardModel);
    _frontRoot->setVisible(showFaceUp);
    _backRoot->setVisible(!showFaceUp);
    if (_highlightSprite != nullptr) {
        _highlightSprite->stopAllActions();
        _highlightSprite->setVisible(showFaceUp);
        _highlightSprite->setOpacity(isInteractive ? 118 : 0);
    }
}

void CardView::setInteractive(bool isInteractive) {
    _isInteractive = isInteractive;
}

void CardView::setCardScale(float cardScale) {
    _cardScale = cardScale;
    setScale(cardScale);
}

void CardView::setTapCallback(const std::function<void(int)>& tapCallback) {
    _tapCallback = tapCallback;
}

void CardView::playMoveTo(
    const cocos2d::Vec2& targetPosition,
    float targetScale,
    float duration,
    const std::function<void()>& onComplete) {
    stopAllActions();
    const float distance = getPosition().distance(targetPosition);
    cocos2d::FiniteTimeAction* moveAction = nullptr;
    if (distance > 80.0f) {
        moveAction = cocos2d::JumpTo::create(duration, targetPosition, std::max(42.0f, distance * 0.14f), 1);
    } else {
        moveAction = cocos2d::MoveTo::create(duration, targetPosition);
    }

    const float targetRotation = targetPosition.x >= getPositionX() ? -6.0f : 6.0f;
    runAction(cocos2d::Sequence::create(
        cocos2d::Spawn::create(
            moveAction,
            cocos2d::EaseSineInOut::create(cocos2d::ScaleTo::create(duration, targetScale)),
            cocos2d::Sequence::create(
                cocos2d::RotateTo::create(duration * 0.45f, targetRotation),
                cocos2d::RotateTo::create(duration * 0.55f, 0.0f),
                nullptr),
            nullptr),
        cocos2d::CallFunc::create([this, targetScale, onComplete]() {
            _cardScale = targetScale;
            setRotation(0.0f);
            if (onComplete) {
                onComplete();
            }
        }),
        nullptr));
}

void CardView::playRevealAnimation() {
    stopActionByTag(101);
    setScale(_cardScale * 0.92f);
    setOpacity(0);
    cocos2d::Action* revealAction = cocos2d::Sequence::create(
        cocos2d::Spawn::create(
            cocos2d::FadeTo::create(0.12f, 255),
            cocos2d::EaseBackOut::create(cocos2d::ScaleTo::create(0.16f, _cardScale * 1.08f)),
            nullptr),
        cocos2d::EaseSineOut::create(cocos2d::ScaleTo::create(0.12f, _cardScale)),
        nullptr);
    revealAction->setTag(101);
    runAction(revealAction);
}

int CardView::getCardId() const {
    return _cardId;
}

void CardView::rebuildFrontFace(const CardModel& cardModel) {
    _frontRoot->removeAllChildren();

    cocos2d::Sprite* cardBaseSprite = cocos2d::Sprite::create(CardResConfig::getCardBasePath());
    cardBaseSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
    cardBaseSprite->setPosition(cocos2d::Vec2(getContentSize().width * 0.5f, getContentSize().height * 0.5f));
    _frontRoot->addChild(cardBaseSprite);

    cocos2d::Sprite* smallNumberSprite =
        cocos2d::Sprite::create(CardResConfig::getSmallNumberPath(cardModel.cardFace, cardModel.cardSuit));
    if (smallNumberSprite != nullptr) {
        smallNumberSprite->setAnchorPoint(cocos2d::Vec2(0.5f, 0.5f));
        smallNumberSprite->setPosition(cocos2d::Vec2(34.0f, 245.0f));
        _frontRoot->addChild(smallNumberSprite);
    }

    cocos2d::Sprite* topRightSuitSprite = cocos2d::Sprite::create(CardResConfig::getSuitPath(cardModel.cardSuit));
    if (topRightSuitSprite != nullptr) {
        topRightSuitSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        topRightSuitSprite->setPosition(cocos2d::Vec2(140.0f, 246.0f));
        topRightSuitSprite->setScale(0.92f);
        _frontRoot->addChild(topRightSuitSprite);
    }

    cocos2d::Sprite* bigNumberSprite =
        cocos2d::Sprite::create(CardResConfig::getBigNumberPath(cardModel.cardFace, cardModel.cardSuit));
    if (bigNumberSprite != nullptr) {
        bigNumberSprite->setAnchorPoint(cocos2d::Vec2::ANCHOR_MIDDLE);
        bigNumberSprite->setPosition(cocos2d::Vec2(91.0f, 122.0f));
        _frontRoot->addChild(bigNumberSprite);
    }
}

bool CardView::isTouchInside(const cocos2d::Touch* touch) const {
    const cocos2d::Vec2 localPoint = convertToNodeSpace(touch->getLocation());
    const cocos2d::Rect localBounds(0.0f, 0.0f, getContentSize().width, getContentSize().height);
    return localBounds.containsPoint(localPoint);
}

}  // namespace cardgame
