# Ns-3_Sumo_Application
 Making the NS-2 file created with Sumo into a network model with NS-3 Lorawan.
## Steps you need to follow
### 1. Step
-- Make sure Ns-3 and LoRaWAN are installed
### 2. Step
-- Download this repo as zip. (Except for experiments.)
### 3.Step
-- Put the ns2mobility.tcl file into the scratch folder inside the ns3.
### 4.Step
-- Put the lastchance.cc file into the scratch folder inside the ns3.
### 5.Step
-- std::string traceFile = "/home/yucel/ns-allinone-3.41/ns-3.41/scratch/ns2mobility.tcl";
Make this part suitable for you.
### 6.Step
-- Paste Script.txt into the terminal. Before doing this make sure you are in the folder where ns3 is installed
### İnformation
Read rapor.pdf for more information. Since this part(rapor) is in Turkish, we think it will be easier to understand.
### Another Information
-- Experiments folder by konuma göre text It talks about preparing the add vector part of the Content of the code from (1000,0,0) to (2500,2500,2500).\n
-- allocator->Add(Vector(1000,0,0)); ( lastchance.cc) ( It explains what happens when this command changes.)( KonumagöreAppPeriods vs AppPeriods and KonumagörenDevices vs nDevices)
-- For detail: read the rapor.pdf
## Contact me
-- email: akgunyucel45@gmail.com


