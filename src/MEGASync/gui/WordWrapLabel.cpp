#include "WordWrapLabel.h"

#include "Utilities.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QTextBlock>
#include <QTextLayout>

#include <algorithm>

/**
 * This label sizes itself to its text: it reports its height through heightForWidth() and is
 * exactly that tall (vertical size policy Fixed), so the layout sizes it correctly on the
 * first pass. It does not vertically centre its own text (QTextBrowser can't); a parent that
 * wants the text centred in a taller row must do so via the layout. Line/height-limited
 * labels also elide their text. See enableHeightForWidth()/heightForWidth()/onAdaptHeight().
 */

//This event is propagated from child to parent, this is why it is used
const QEvent::Type WordWrapLabel::HeightAdapted = QEvent::WhatsThisClicked;

WordWrapLabel::WordWrapLabel(QWidget* parent):
    QTextBrowser(parent),
    mLinkActivated(false),
    mMaxHeight(-1),
    mMaxLines(-1),
    mFormat(Qt::PlainText),
    mParentHeight(-1),
    mKeepParentCursor(true),
    mAutoManageUrl(true)
{
    setFrameStyle(QFrame::NoFrame);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    setCursor(parent->cursor());
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setOpenLinks(false);

    parent->installEventFilter(this);
    viewport()->installEventFilter(this);

    document()->setDocumentMargin(0);

    // The label sizes itself to its text via heightForWidth() (see below). The .ui may
    // override the size policy afterwards; setText()/resetSizeLimits() re-assert it.
    enableHeightForWidth();

    connect(this, &WordWrapLabel::anchorClicked, this, &WordWrapLabel::onLinkActivated);

    //Timer to avoid multiple height adaptations if you change the limit type
    //The height adaptation will be done in the following event loop
    mAdaptHeightTimer.setSingleShot(true);
    mAdaptHeightTimer.setInterval(0);
    connect(&mAdaptHeightTimer, &QTimer::timeout, this, [this](){onAdaptHeight();});
}

void WordWrapLabel::setMaximumLines(int8_t lines)
{
    //Don´t use two limits at the same time
    Q_ASSERT_X(mMaxHeight == -1, "WordWrapLabel", "Use resetSizeLimits before using this method");
    mMaxLines = lines;
    enableHeightForWidth();
    mAdaptHeightTimer.start();
}

void WordWrapLabel::setMaximumHeight(int maxHeight)
{
    //Don´t use two limits at the same time
    Q_ASSERT_X(mMaxLines == -1, "WordWrapLabel", "Use resetSizeLimits before using this method");
    mMaxHeight = maxHeight;
    enableHeightForWidth();
    mAdaptHeightTimer.start();
}

void WordWrapLabel::resetSizeLimits()
{
    mMaxHeight = -1;
    mMaxLines = -1;
    QTextEdit::setText(mText);
    enableHeightForWidth();
    mAdaptHeightTimer.start();
}

void WordWrapLabel::setText(const QString& text)
{
    if (mText != text)
    {
        mText = text;

        if (mFormat == Qt::PlainText)
        {
            QTextBrowser::setPlainText(mText);
        }
        else
        {
            QTextBrowser::setText(mText);
        }

        // The label's height comes from heightForWidth(); make sure the policy is in place
        // even if the .ui overrode it. Line/height-limited labels still elide their text in
        // onAdaptHeight(), but their height is taken from heightForWidth() too.
        enableHeightForWidth();

        onAdaptHeight();
    }
}

void WordWrapLabel::setTextFormat(Qt::TextFormat format)
{
    mFormat = format;
}

void WordWrapLabel::setKeepParentCursor(bool newValue)
{
    mKeepParentCursor = newValue;
}

void WordWrapLabel::onAdaptHeight(bool parentConstrained)
{
    if(mText.isEmpty())
    {
        return;
    }

    //Don´t do line counts if we don´t want it to be limited by height or number of lines
    //This saves time as we don´t need the number of lines
    if(mMaxLines < 0 && mMaxHeight < 0)
    {
        // The height is now provided by heightForWidth(), so the layout sizes this label
        // correctly on its first pass (even while hidden) without needing to receive a resize
        // event first. We only need to tell the layout to re-query it when the reported height
        // actually changed (e.g. new text or new width).
        requestHeightUpdateIfChanged();
    }
    //TODO check for names with \n
    else
    {
        QTextLayout* textLayout = document()->firstBlock().layout();
        if (!textLayout)
        {
            return;
        }

        textLayout->beginLayout();

        int processedStringLength(0);
        int lineCounter(0);
        int fontHeight = fontMetrics().lineSpacing();

        //This while is break when a new line is invalid
        //or we need to elide the last line
        while (true)
        {
            //Check if this is the last line
            if ((mMaxHeight > 0 && ((lineCounter + 1) * fontHeight) >= mMaxHeight) ||
                (mMaxLines > 0 && (lineCounter + 1 == mMaxLines)) ||
                (parentConstrained && ((lineCounter + 2) * fontHeight) * devicePixelRatio() >= visibleRegion().boundingRect().height()))
            {
                auto modifiedText(mText);
                auto textNotToElide(modifiedText.left(processedStringLength));
                auto textToElide(modifiedText.remove(0, processedStringLength));
                if (!textToElide.isEmpty())
                {
                    auto elidedText(fontMetrics().elidedText(textToElide, Qt::ElideMiddle, width()));

                    //Elide line height
                    auto elideLine = textLayout->createLine();
                    if (!elideLine.isValid())
                    {
                        break;
                    }
                    elideLine.setLineWidth(width());
                    lineCounter++;

                    elidedText.prepend(textNotToElide);

                    //QTextEdit::setText removes the textLayout so its important to not remove these lines
                    textLayout->endLayout();
                    textLayout = nullptr;

                    if (mFormat == Qt::PlainText)
                    {
                        QTextBrowser::setPlainText(elidedText);
                    }
                    else
                    {
                        QTextBrowser::setText(elidedText);
                    }

                    if(elidedText != mText)
                    {
                        setToolTip(stripHtmlTags(mText));
                    }

                    break;
                }
            }

            auto line = textLayout->createLine();
            if (!line.isValid())
            {
                break;
            }
            line.setLineWidth(width());
            lineCounter++;
            processedStringLength += line.textLength();
        }

        if (textLayout)
        {
            textLayout->endLayout();
        }

        setLineWrapColumnOrWidth(lineWrapColumnOrWidth());

        // The (possibly elided) text is now set; its height is provided by heightForWidth(),
        // so the layout sizes us correctly on its first pass instead of one resize later.
        requestHeightUpdateIfChanged();
    }
}

void WordWrapLabel::requestHeightUpdateIfChanged()
{
    // Re-query the layout only when the height we'd report actually changed. Compare with a
    // 1px tolerance: sanitizeHeight() rounds the reported height to an even number, so a
    // strict != against the realised height() could ping-pong on borderline-wrapping text
    // (height adapt -> dialog adjustSize -> resize -> height adapt ...).
    if (qAbs(heightForWidth(width()) - height()) > 1)
    {
        updateGeometry();
        qApp->postEvent(this, new QEvent(HeightAdapted));
    }
}

void WordWrapLabel::enableHeightForWidth()
{
    QSizePolicy policy = sizePolicy();
    if (!policy.hasHeightForWidth() || policy.verticalPolicy() != QSizePolicy::Fixed)
    {
        // The height comes from heightForWidth(), so the label is exactly as tall as its
        // text (vertical policy Fixed). It does NOT try to fill a taller row and centre its
        // text itself: QTextBrowser always top-aligns text vertically, so any centring has
        // to be done by the parent layout (e.g. with surrounding stretches).
        policy.setVerticalPolicy(QSizePolicy::Fixed);
        policy.setHeightForWidth(true);
        setSizePolicy(policy);
    }
}

int WordWrapLabel::heightForWidth(int width) const
{
    if (mText.isEmpty() || width <= 0)
    {
        return 0;
    }

    // Measure on a scratch document so we never disturb the visible one. This is a pure
    // function of (text, font, width), which is exactly what the layout needs to size us.
    mMeasureDoc.setDefaultFont(font());
    mMeasureDoc.setDocumentMargin(document()->documentMargin());
    if (mFormat == Qt::PlainText)
    {
        mMeasureDoc.setPlainText(mText);
    }
    else
    {
        mMeasureDoc.setHtml(mText);
    }
    mMeasureDoc.setTextWidth(width);

    int textHeight = mMeasureDoc.size().toSize().height();

    // Honour the same limits the reactive (line/height-limited) path applies.
    if (mMaxLines > 0)
    {
        textHeight =
            (std::min)(textHeight, static_cast<int>(mMaxLines) * fontMetrics().lineSpacing());
    }
    if (mMaxHeight > 0)
    {
        textHeight = (std::min)(textHeight, mMaxHeight);
    }

    sanitizeHeight(textHeight);
    return textHeight;
}

QSize WordWrapLabel::sizeHint() const
{
    QSize hint = QTextBrowser::sizeHint();
    hint.setHeight(heightForWidth(width() > 0 ? width() : hint.width()));
    return hint;
}

QSize WordWrapLabel::minimumSizeHint() const
{
    QSize hint = QTextBrowser::minimumSizeHint();
    hint.setHeight(heightForWidth(width() > 0 ? width() : hint.width()));
    return hint;
}

void WordWrapLabel::resizeEvent(QResizeEvent* event)
{
    /*
      * If the widget has been resized then the size hint will
      * also have changed.  Call updateGeometry to make sure
      * any layouts are notified of the change.
      */
    updateGeometry();

    //Here we dont use the timer as we want to do it right now, and not in the following event loop
    if(!mText.isEmpty())
    {
        onAdaptHeight();
    }


    QTextBrowser::resizeEvent(event);
}

bool WordWrapLabel::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::CursorChange)
    {
        if (mKeepParentCursor && viewport()->cursor().shape() != parentWidget()->cursor().shape())
        {
            setCursor(parentWidget()->cursor());
        }
    }
    if(event->type() == QEvent::Resize && obj == parent())
    {
        updateGeometry();
        onAdaptHeight(parentWidget()->height() < mParentHeight);
        mParentHeight = parentWidget()->height();
    }
    return QTextBrowser::eventFilter(obj, event);
}

void WordWrapLabel::mouseReleaseEvent(QMouseEvent* ev)
{
    QTextBrowser::mousePressEvent(ev);

    //In order to propagate the mouse release event
    //Otherwise, the QTextBrowser hijacks it even if you don´t press of the link
    if(!mLinkActivated)
    {
        QWidget::mousePressEvent(ev);
    }

    //Reset for next click
    mLinkActivated = false;
}

void WordWrapLabel::onLinkActivated(const QUrl& link)
{
    mLinkActivated = true;
    if (mAutoManageUrl)
    {
        Utilities::openUrl(link);
    }
}

void WordWrapLabel::setCursor(const QCursor& cursor)
{
    QTextEdit::setCursor(cursor);
    viewport()->setCursor(cursor);
}

void WordWrapLabel::sanitizeHeight(int& height) const
{
    // We don´t want odd numbers
    if (height % 2 != 0)
    {
        height += 1;
    }
}

void WordWrapLabel::setAutoManageUrl(bool newValue)
{
    mAutoManageUrl = newValue;
}

QString WordWrapLabel::stripHtmlTags(const QString &text)
{
    mTextDocument.setHtml(text);
    return mTextDocument.toPlainText();
}
