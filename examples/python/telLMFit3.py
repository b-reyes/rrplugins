import roadrunner
import numpy as np
import matplotlib.pyplot as plt
import scipy.stats as stats
import telplugins as tel
import ctypes as ct

try:
        
    sbmlmPluginodel = "sbml_test_0001.xml" 
    # Create a roadrunner instance and create some data
    rr = roadrunner.RoadRunner()
    rr.load(sbmlmPluginodel)    
    data = rr.simulate(0, 10, 15000) # Want 512 points

    #Add noise to the data
    noisePlugin = tel.Plugin ("tel_add_noise")

    # Get the dataseries from data returned by roadrunner
    d = tel.getDataSeries (data)

    # Assign the dataseries to the plugin inputdata
    noisePlugin.InputData = d

    # Set parameter for the 'size' of the noise
    noisePlugin.Sigma = 3.e-6

    # Add the noise
    noisePlugin.execute()
    fName = 'testData3.dat'
    noisePlugin.InputData.writeDataSeries (fName)       
    #===================================================================
    
    lmPlugin = tel.Plugin ("tel_lm")
    experimentalData = tel.DataSeries.readDataSeries (fName)    
       
    lmPlugin.ExperimentalData = experimentalData;
    lmPlugin.SBML = lmPlugin.readAllText(sbmlmPluginodel)
    
    # Add the parameters that we're going to fit and the initial value
    lmPlugin.InputParameterList = ["k1", .1]   
    lmPlugin.FittedDataSelectionList = "[S1] [S2]"
    lmPlugin.ExperimentalDataSelectionList = "[S1] [S2]"
        
    # Execute lmPluginfit plugin
    res = lmPlugin.execute()
        
    # Get the residuals    
    residuals  = lmPlugin.Residuals.toNumpy               
    residuals = residuals[:,[1,2]]
    
    #Plot as a histogram
    plt.hist(residuals, 50, normed=True)
    plt.show()
        
    stdResiduals = lmPlugin.StandardizedResiduals.toNumpy
    stdResiduals = stdResiduals[:,[1,2]]            
    plt.hist(stdResiduals, 50, normed=True)
    plt.show()
          
    #Plot normal probability plots
    probPlots = lmPlugin.NormalProbabilityOfResiduals.toNumpy
    print probPlots
    
    x1 = probPlots[:,[0]]
    y1 = probPlots[:,[1]]    
        
    x2 = probPlots[:,[2]]
    y2 = probPlots[:,[3]]    
    
    lineX = np.arange(-5, 5, .1)
    lineY = lineX
    plt.plot(lineX, lineY, '-r')         
    plt.plot(x1,y1, 'o')
    plt.plot(x2,y2, 'x')    
    #plt.scatter(x,y)    
    plt.show()
        
except Exception as e:
    print 'Exception: ' + `e`

