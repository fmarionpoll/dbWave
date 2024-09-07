# dbWave

**dbwave** is a program routinely used in our laboratory to acquire and analyze extracellular data from olfactory and taste sensilla of insects. 
During an experimental session, it collects data and stores them on disk, along with information describing the experiment.
After the experiment, recordings are displayed and analyzed interactively, to detect and sort spikes, and to export data and the results under different formats. 

**Data acquisition**: dbWave is interfaced to [Data Translation](http://www.datatranslation.eu/) cards to sample electrical data on several channels (1 to 8 or 16), convert them into numbers (12 or 16 bits) by sampling them at a frequency usually of 10 kHz and store them into a binary file along with a descriptor header (**.dat files**).
It also communicates to Axon Instrument amplifiers (CyberAmp 320) and to Alligator Technology amplifiers.
Depending on the Data Translation card model, it may also drive digital or output lines. 
Usually, data acquisition bouts are triggered by an external TTL signal.

**Data analysis**: dbWave2 reads binary data files stored under different formats.
Typically, it is used in our laboratory to detect and sort action potentials from one or several channels.
Spikes detected from a recording are stored as arrays of recording bouts(typically 60 data points), together with the time and a class number int a spike file (**.spk files**).

**Database**: dbWave2 collects information about each recording (time, stimulus, concentration, insect, sensillum, etc) either directly or from the user, to build a database (**.mdb files**). 
This database allows the user to interactively select files of interest based on multiple criteria.

**Export**: Data and results can be exported to various formats, including as Excel tables or figures. 


# Getting Started
TODO: Guide users through getting your code up and running on their own system. 
## 1.	Installation process
## 2.	Software dependencies
## 3.	Latest releases
## 4.	API references

# Build and Test
TODO: Describe and show how to build your code and run the tests. 

# Contribute
TODO: Explain how other users and developers can contribute to make your code better. 

If you want to learn more about creating good readme files then refer the following [guidelines](https://www.visualstudio.com/en-us/docs/git/create-a-readme). You can also seek inspiration from the below readme files:
- [ASP.NET Core](https://github.com/aspnet/Home)
- [Visual Studio Code](https://github.com/Microsoft/vscode)
- [Chakra Core](https://github.com/Microsoft/ChakraCore)
