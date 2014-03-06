#pragma hdrstop
#include "nmObjectiveFunction.h"
#include "nmNelderMead.h"
#include "telMathUtils.h"
#include "telProperties.h"
//---------------------------------------------------------------------------

using namespace tlp;

double NelderMeadObjectiveFunction(double par[], const void* userData)
{
    NelderMead&         NMP     = *((NelderMead*) userData);
    TelluriumData&      expData = NMP.mExperimentalData;
    RoadRunner*         rr      = NMP.getRoadRunner();
    Plugin&             chi     = *(NMP.getChiSquarePlugin());
    if(!rr)
    {
        throw(Exception("RoadRunner is NULL in Nelder-Mead objective function"));
    }
    //Reset roadrunner
    rr->reset();

    //Get current parameter values
    Properties* paras = (Properties*) NMP.mInputParameterList.getValueHandle();
    int nrOfParameters = paras->count();

    for(int i =0; i < nrOfParameters; i++)
    {
        PropertyBase* para = paras->getPropertyAt(i);
        double parValue = par[i];
        para->setValue( &parValue  );
        if(!rr->setValue(para->getName(), * (double*) para->getValueHandle()))
        {
            throw(Exception("Failed setting value of RoadRunner parameter"));
        }

    }

    rr::SimulateOptions opt;
    opt.start       = expData.getTimeStart();
    opt.duration    = expData.getTimeEnd() - expData.getTimeStart();
    opt.steps       = expData.rSize() -1;

    //Simulate
    TelluriumData   simData(rr->simulate(&opt));

    StringList* species = (StringList*) NMP.mExperimentalDataSelectionList.getValueHandle();
    int nrOfSpecies = species->Count();

    //Calculate residuals
    vector<double> residuals(simData.rSize() * nrOfSpecies);

    int resIndex = 0;
    for(int specie = 0; specie < nrOfSpecies; specie++)
    {
        for(int timePoint = 0; timePoint < simData.rSize(); timePoint++)
        {
            residuals[resIndex] = expData(timePoint, specie + 1) - simData(timePoint, specie); //+1 because of time
            resIndex ++;
        }
    }
    //Calculate Norm


    double norm = getEuclideanNorm(residuals);


    //Call OnProgress
    return norm;
}
