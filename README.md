# VAMA

The Voice Assistant Modular Adapter takes advantage of the popularity of smart devices and the ongoing advancement in voice processing technology. 
The system will allow the disabled community to gain all of the benefits of having smart devices without having to replace all of their appliances. 
This means that the Voice Assistant Modular Adapter provides a low cost to entry for smart home integration as well giving the disabled community a means to operate their daily-use devices with only their voice.
The system works by using Alexa’s voice processing technology as well as Amazon skills that were written for our system to drive a Heroku server that keeps track of all of the different appliances in the network. 
Microcontrollers attached to each appliance not only ensure the functionality of each individual appliance but also keep track of their current state and relay that information back to the server. If the state of any appliance is altered on the server, whether through a voice command or manually on the website, the microcontroller will execute that state change for the attached appliance.  


## Files
The files in the Github represent all code from all locations from the project. 

## Webserver
The folder "Voice-Assistant-Modular-Adapter-ECEN403-SeniorDesign-master" contains the entire php and NOSQL script that is hosted live on vama.herokuapp.com.
To run the webserver a simple LAMP localhost will work or any php web hosts (like heroku or aws) will do.


## Alexa
"VAMA.JSON" contains the JSON information for creating the Alexa skills. This informs Alexa how to understand voice commands.
"index.js" Is the javascript for the aws lambda server which processes the skill and sends the information to the heroku server.
Both of these files are normally found in the developer console of an Alexa skills project at developer.amazon.com

## Microcontrollers
The files related to the microcontrollers are: 
"IRController.ino"
"ServerMotorController.ino"
"ServerMotorControlUpdatedAJSON.ino"

These all do similar functions. They recieve JSON information from the heroku server via GET requests and compare that data to the current values
The code then activates either an IR emitter or motor to drive the physical device changes. 
