#pragma hdrstop
#include <iomanip>
#include "rr/rrException.h"
#include "rr/rrLogger.h"
#include "rr/rrRoadRunnerData.h"
#include "telUtils.h"
#include "telStringUtils.h"
#include "telIniFile.h"
#include "Poco/TemporaryFile.h"
#include "telTelluriumData.h"

//---------------------------------------------------------------------------
using namespace std;

using rr::Logger;
using rr::CoreException;
namespace tlp
{

TelluriumData::TelluriumData(const int& rSize, const int& cSize ) 
    :        
    mTimePrecision(6),
    mDataPrecision(16)
{
    if(cSize && rSize)
    {
        allocate(rSize, cSize);
    }
}

TelluriumData::TelluriumData(const StringList& colNames, const DoubleMatrix& theData) 
    :        
    mTimePrecision(6),
    mDataPrecision(16),
    mColumnNames(colNames),
    mTheData(theData)
{}

TelluriumData::~TelluriumData()
{}

void TelluriumData::clear()
{
    mColumnNames.clear();
    mTheData.resize(0,0);
    mWeights.resize(0,0);
}

int TelluriumData::cSize() const
{
    return mTheData.CSize();
}

int TelluriumData::rSize() const
{
    return mTheData.RSize();
}

double TelluriumData::getTimeStart() const
{
    //Find time column
    int timeCol = indexOf(mColumnNames, "time");
    if(timeCol != -1)
    {
        return mTheData(0,timeCol);
    }
    return gDoubleNaN;
}

double TelluriumData::getTimeEnd() const
{
    //Find time column
    int timeCol = indexOf(mColumnNames, "time");
    if(timeCol != -1)
    {
        return mTheData(rSize() -1 ,timeCol);
    }
    return gDoubleNaN;
}

void TelluriumData::setName(const string& name)
{
    mName = name;
}

TelluriumData& TelluriumData::operator= (const TelluriumData& rhs)
{
    if(this == &rhs)
    {
        return *this;
    }

    mTheData = rhs.mTheData;
    mWeights = rhs.mWeights;
    mColumnNames = rhs.mColumnNames;
    return *this;
}

TelluriumData& TelluriumData::operator= (const rr::RoadRunnerData& rhs)
{
    mTheData = rhs.getData();
    mWeights = rhs.getWeights();
    mColumnNames = rhs.getColumnNames();
    return *this;
}

void TelluriumData::allocateWeights()
{
    //Create matrix with weights... initialize all elements to 1
    mWeights.Allocate(mTheData.RSize(), mTheData.CSize());
    for(int i = 0; i < rSize(); i++)
    {
        for(int j = 0; j < cSize(); j++)
        {
            mWeights(i,j) = 1;
        }
    }
}

bool TelluriumData::append(const TelluriumData& data)
{
    //When appending data, the number of rows have to match with current data
    if(mTheData.RSize() > 0)
    {
        if(data.rSize() != rSize())
        {
            return false;
        }
    }
    else
    {
        (*this) = data;
        return true;
    }

    int currColSize = cSize();

    TelluriumData temp(mColumnNames, mTheData);

    int newCSize = cSize() + data.cSize();
    mTheData.resize(data.rSize(), newCSize );

    for(int row = 0; row < temp.rSize(); row++)
    {
        for( int col = 0; col < temp.cSize(); col++)
        {
            mTheData(row, col) = temp(row, col);
        }
    }

    for(int row = 0; row < mTheData.RSize(); row++)
    {
        for(int col = 0; col < data.cSize(); col++)
        {
            mTheData(row, col + currColSize) = data(row, col);
        }
    }

    for(int col = 0; col < data.cSize(); col++)
    {
        mColumnNames.add(data.getColumnName(col));
    }
    return true;
}

const StringList& TelluriumData::getColumnNames() const
{
    return mColumnNames;
}

bool TelluriumData::isFirstColumnTime() const
{
    if(mColumnNames.size() > 0)
    {
        return compareNoCase(mColumnNames[0], "Time") == 0;
    }
    return false;

}

string TelluriumData::getColumnName(const int col) const
{
    if(col < mColumnNames.size())
    {
        return mColumnNames[col];
    }

    return "Bad Column..";
}

int TelluriumData::getColumnIndex(const string& colName) const
{
    return indexOf(mColumnNames, colName);
}

pair<int,int> TelluriumData::dimension() const
{
    return pair<int,int>(mTheData.RSize(), mTheData.CSize());
}

string TelluriumData::getName() const
{
    return mName;
}

void TelluriumData::setTimeDataPrecision(const int& prec)
{
    mTimePrecision = prec;
}

void TelluriumData::setDataPrecision(const int& prec)
{
    mDataPrecision = prec;
}

string TelluriumData::getColumnNamesAsString() const
{
    string lbls;
    for(int i = 0; i < mColumnNames.size(); i++)
    {
        lbls.append(mColumnNames[i]);
        if(i < mColumnNames.size() -1)
        {
            lbls.append(",");
        }
    }
    return lbls;
}

void TelluriumData::allocate(const int& cSize, const int& rSize)
{
    mTheData.Allocate(cSize, rSize);
}

//=========== OPERATORS
double& TelluriumData::operator() (const unsigned& row, const unsigned& col)
{
    return mTheData(row,col);
}

bool TelluriumData::hasWeights() const
{
    return (mWeights.size() > 0) ? true : false;
}


double TelluriumData::getDataElement(int row, int col)
{
    return mTheData(row,col);    
}

void   TelluriumData::setDataElement(int row, int col, double value)
{
    mTheData(row,col) = value;
}

double TelluriumData::getWeight(int row, int col) const
{
    return mWeights(row, col);
}

void TelluriumData::setWeight(int row, int col, double value)
{
    mWeights(row, col) = value;
}

double TelluriumData::operator() (const unsigned& row, const unsigned& col) const
{
    return mTheData(row,col);
}

bool TelluriumData::setColumnName(int index, const string& name)
{
    mColumnNames[index] = name;
    return true;
}

bool TelluriumData::setColumnNames(const StringList& colNames)
{
    if(colNames.size() != mTheData.CSize())
    {
        return false;
    }
    else
    {
        mColumnNames = colNames;
        return true;
    }    
}


void TelluriumData::reSize(int rows, int cols)
{
    mTheData.Allocate(rows, cols);
}

void TelluriumData::setData(const DoubleMatrix& theData)
{
    mTheData = theData;
    Log(Logger::LOG_DEBUG) << "Simulation Data =========== \n" << mTheData;
    check();
}

bool TelluriumData::check() const
{
    if(mTheData.CSize() != mColumnNames.size())
    {
        Log(Logger::LOG_ERROR)<<"Number of columns ("<<mTheData.CSize()<<") in simulation data is not equal to number of columns in column header ("<<mColumnNames.size()<<")";
        return false;
    }
    return true;
}

bool TelluriumData::loadSimpleFormat(const string& fName)
{
    if(!fileExists(fName))
    {
        return false;
    }

    vector<string> lines = getLinesInFile(fName.c_str());
    if(!lines.size())
    {
        Log(Logger::LOG_ERROR)<<"Failed reading/opening file "<<fName;
        return false;
    }

    mColumnNames = splitString(lines[0], ",");
    Log(lInfo) << toString(mColumnNames);

    mTheData.resize(lines.size() -1, mColumnNames.size());

    for(int i = 0; i < mTheData.RSize(); i++)
    {
        vector<string> aLine = splitString(lines[i+1], ", ");
        for(int j = 0; j < aLine.size(); j++)
        {
            mTheData(i,j) = toDouble(aLine[j]);
        }
    }

    return true;
}

bool TelluriumData::writeTo(const string& fileName) const
{
    ofstream aFile(fileName.c_str());
    if(!aFile)
    {
        stringstream s;
        s<<"Failed opening file: "<<fileName;
        Log(Logger::LOG_ERROR)<<s.str();
        throw(rr::Exception(s.str()));        
    }
 
    if(!check())
    {

        stringstream s;
        s<<"Can't write data.. the dimension of the header don't agree with nr of cols of data";
        Log(Logger::LOG_ERROR)<<s.str();
        throw(rr::Exception(s.str()));                        
    }

    aFile<<(*this);
    aFile.close();
    return true;
}

bool TelluriumData::readFrom(const string& fileName)
{
    ifstream aFile(fileName.c_str());
    if(!aFile)
    {
        stringstream s;
        s<<"Failed opening file: "<<fileName;
        Log(Logger::LOG_ERROR)<<s.str();
        throw(rr::Exception(s.str()));        
    }
    
    aFile >> (*this); 
    aFile.close();
    return true;
}

ostream& operator << (ostream& ss, const TelluriumData& data)
{
    //Check that the dimensions of col header and data is ok
    if(!data.check())
    {
        Log(Logger::LOG_ERROR)<<"Can't write data.. the dimension of the header don't agree with nr of cols of data";
        return ss;
    }

    ss<<"[INFO]"<<endl;
    ss<<"DATA_FORMAT_VERSION=1.0"   <<endl;
    ss<<"CREATOR=tellurium"      <<endl;
    ss<<"NUMBER_OF_COLS="            <<data.cSize()<<endl;
    ss<<"NUMBER_OF_ROWS="            <<data.rSize()<<endl;
    ss<<"COLUMN_HEADERS="            <<data.getColumnNamesAsString()<<endl;

    ss<<endl;
    ss<<"[DATA]"<<endl;
    //Then the data
    for(u_int row = 0; row < data.mTheData.RSize(); row++)
    {
        for(u_int col = 0; col < data.mTheData.CSize(); col++)
        {
            if(col == 0)
            {
                ss<<setprecision(data.mTimePrecision)<<data.mTheData(row, col);
            }
            else
            {
                ss<<setprecision(data.mDataPrecision)<<data.mTheData(row, col);
            }

            if(col <data.mTheData.CSize() -1)
            {
                ss << ",";
            }
            else
            {
                ss << endl;
            }
        }
    }

    if(data.mWeights.isAllocated())
    {
        //Write weights section
        ss<<endl;
        ss<<"[WEIGHTS]"<<endl;

        //Then the data
        for(u_int row = 0; row < data.mWeights.RSize(); row++)
        {
            for(u_int col = 0; col < data.mWeights.CSize(); col++)
            {
                if(col == 0)
                {
                    ss<<setprecision(data.mTimePrecision)<<data.mWeights(row, col);
                }
                else
                {
                    ss<<setprecision(data.mDataPrecision)<<data.mWeights(row, col);
                }

                if(col <data.mTheData.CSize() -1)
                {
                    ss << ",";
                }
                else
                {
                    ss << endl;
                }
            }
        }
    }
    return ss;
}

//Stream data from a file
istream& operator >> (istream& ss, TelluriumData& data)
{
    //Read in all lines into a string
    std::string oneLine((std::istreambuf_iterator<char>(ss)), std::istreambuf_iterator<char>());

    //This is pretty inefficient
    string tempFileName = joinPath(getUsersTempDataFolder(), "rrTempFile.dat");
    ofstream tempFile(tempFileName.c_str());

    tempFile << oneLine;
    tempFile.close();

    //Create a iniFile object and parse the data
    IniFile ini(tempFileName, true);

    IniSection* infoSection = ini.GetSection("INFO");
    if(!infoSection)
    {
        stringstream s;
        s<<"RoadRunnder data file is missing section: [INFO]. Exiting reading file...";
        Log(Logger::LOG_ERROR)<<s.str();
        throw(rr::Exception(s.str()));        
    }

    //Setup header
    IniKey* colNamesK = infoSection->GetKey("COLUMN_HEADERS");

    if(!colNamesK)
    {
        stringstream s;
        s<<"RoadRunnder data file is missing ini key: COLUMN_HEADERS. Exiting reading file...";
        Log(Logger::LOG_ERROR)<<s.str();
        throw(rr::Exception(s.str()));                
    }

    StringList colNames(splitString(colNamesK->mValue, ",{}"));
    

    //Read number of cols and rows and setup data
    IniKey* aKey1 = infoSection->GetKey("NUMBER_OF_COLS");
    IniKey* aKey2 = infoSection->GetKey("NUMBER_OF_ROWS");
    if(!aKey1 || !aKey2)
    {
       stringstream s;
       s<<"RoadRunnder data file is missing ini key: NUMBER_OF_COLS and/or NUMBER_OF_ROWS. Exiting reading file...";       
       Log(Logger::LOG_ERROR)<<s.str();
       throw(rr::Exception(s.str()));                
    }

    int rDim = aKey2->AsInt();
    int cDim = aKey1->AsInt();
    
    //Check that number of col names correspond to cDim
    if(colNames.size() != aKey1->AsInt())
    {
       stringstream s;
       s<<"Tellurium data format error: NUMBER_OF_COLS ("<<cDim<<") don't correspond to number of column headers ("<<colNames.size()<<").";       
       Log(Logger::LOG_ERROR)<<s.str();
       throw(rr::Exception(s.str()));                 
    }
    
    
    data.reSize(rDim, cDim);
    data.setColumnNames(colNames);
    //get data section
    IniSection* dataSection = ini.GetSection("DATA");
    if(!dataSection)
    {
        stringstream s;
        s<<"RoadRunnder data file is missing ini section: DATA. Exiting reading file...";
        Log(Logger::LOG_ERROR)<<s.str();
        throw(rr::Exception(s.str()));                
    }

    vector<string> lines = splitString(dataSection->GetNonKeysAsString(), "\n");
    for(int row = 0; row < lines.size(); row++)
    {
        string oneLine = lines[row];
        vector<string> aLine = splitString(oneLine, ',');
        if(aLine.size() != cDim)
        {
            throw(CoreException("Bad Tellurium data in [DATA] section"));
        }

        for(int col = 0; col < cDim; col++)
        {
            Log(lDebug5)<<"Word "<<aLine[col];
            double value = toDouble(trim(aLine[col],' '));
            data(row, col) = value;
        }
    }

    //Weights ??

    IniSection* weightsSection = ini.GetSection("WEIGHTS");
    if(!weightsSection)    //Optional
    {
        Log(lDebug)<<"RoadRunnder data file is missing section: [WEIGHTS]. ";
        return ss;
    }
    data.mWeights.Allocate(rDim, cDim);

    lines = splitString(weightsSection->GetNonKeysAsString(), "\n");
    for(int row = 0; row < lines.size(); row++)
    {
        string oneLine = lines[row];
        vector<string> aLine  = splitString(oneLine, ',');
        if(aLine.size() != cDim)
        {
            throw(CoreException("Bad Tellurium data in [WEIGHTS] section."));
        }

        for(int col = 0; col < cDim; col++)
        {
            Log(lDebug5)<<"Word "<<aLine[col];
            double value = toDouble(aLine[col]);
            data.mWeights(row, col) = value;
        }
    }

    return ss;
}

const DoubleMatrix& TelluriumData::getData() const
{
    return mTheData;
}

const DoubleMatrix& TelluriumData::getWeights() const
{
    return mWeights;
}

}//namespace