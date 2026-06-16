#ifndef WORDWRAPLABEL_H
#define WORDWRAPLABEL_H

#include <QEvent>
#include <QTextBrowser>
#include <QTimer>

class WordWrapLabel : public QTextBrowser
{
    Q_OBJECT

public:
    static const QEvent::Type HeightAdapted;

    WordWrapLabel(QWidget *parent);

    void setMaximumLines(int8_t lines);
    void setMaximumHeight(int maxHeight);
    void resetSizeLimits();

    void setText(const QString& text);

    //Try not to use maxLines/maxHeight with rich text strings, as it could potentially remove the hmtl tags when eliding
    void setTextFormat(Qt::TextFormat format);

    void setKeepParentCursor(bool newValue);
    void setAutoManageUrl(bool newValue);

    // The height of this label is a function of its width. Report it through the
    // height-for-width contract so the layout sizes the label correctly on the very first
    // pass, instead of having to receive a resize event first and then re-adapt (which made
    // dialogs briefly appear too tall before shrinking to their final size).
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int heightForWidth(int width) const override;

protected:
    void resizeEvent(QResizeEvent *e) override;
    bool eventFilter(QObject* obj, QEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    QString stripHtmlTags(const QString &text);

private slots:
    void onLinkActivated(const QUrl& link);
    void onAdaptHeight(bool parentConstrained = false);

private:
    void setCursor(const QCursor& cursor);
    void sanitizeHeight(int& height) const;
    void enableHeightForWidth();
    void requestHeightUpdateIfChanged();

    bool mLinkActivated;
    int mMaxHeight;
    int8_t mMaxLines;
    QString mText;
    Qt::TextFormat mFormat;
    QTimer mAdaptHeightTimer;
    int mParentHeight;
    QTextDocument mTextDocument; // This is only to remove html tags from tooltips
    mutable QTextDocument mMeasureDoc; // Scratch document used to measure heightForWidth()
    bool mKeepParentCursor;
    bool mAutoManageUrl;
};

#endif // WORDWRAPLABEL_H
