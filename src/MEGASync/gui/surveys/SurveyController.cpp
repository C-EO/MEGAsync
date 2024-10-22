#include "SurveyController.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "QmlManager.h"

SurveyController::SurveyController(QObject* parent):
    QObject(parent),
    mSurveys(std::make_shared<Surveys>(nullptr))
{}

void SurveyController::onRequestFinish(mega::MegaRequest* request, mega::MegaError* error)
{
    switch (request->getType())
    {
        case mega::MegaRequest::TYPE_GET_ACTIVE_SURVEY_TRIGGER_ACTIONS:
        {
            break;
        }
        case mega::MegaRequest::TYPE_GET_SURVEY:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

void SurveyController::registerQmlRootContextProperties()
{
    QmlManager::instance()->setRootContextProperty(mSurveys.get());
}

void SurveyController::updateCurrentAnwer(int response, const QString& comment)
{
    auto survey(mSurveys->currentSurvey());
    survey->setAnswerData(response, comment);
}

void SurveyController::submitSurvey()
{
    auto survey(mSurveys->currentSurvey());
    int response(survey->answer().response());
    MegaSyncApp->getMegaApi()->answerSurvey(
        survey->handle(),
        mSurveys->currentSurveyId(),
        std::to_string(response).c_str(),
        response == 1 || response == 2 ? survey->answer().comment().toUtf8().data() : nullptr);

    emit surveySubmitted();
}
