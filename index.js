/* *
 * This sample demonstrates handling intents from an Alexa skill using the Alexa Skills Kit SDK (v2).
 * Please visit https://alexa.design/cookbook for additional examples on implementing slots, dialog management,
 * session persistence, api calls, and more.
 * */
const Alexa = require('ask-sdk-core');
var axios = require("axios").default;

const LaunchRequestHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'LaunchRequest';
    },
    handle(handlerInput) {
        const speakOutput = 'Welcome, you can say Hello or Help. Which would you like to try?';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

const HelloWorldIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'HelloWorldIntent';
    },
    handle(handlerInput) {
        const speakOutput = 'Hello World!';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            //.reprompt('add a reprompt if you want to keep the session open for the user to respond')
            .getResponse();
    }
};

const HelpIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.HelpIntent';
    },
    handle(handlerInput) {
        const speakOutput = 'You can say hello to me! How can I help?';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

const CancelAndStopIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && (Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.CancelIntent'
                || Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.StopIntent');
    },
    handle(handlerInput) {
        const speakOutput = 'Goodbye!';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .getResponse();
    }
};
/* *
 * FallbackIntent triggers when a customer says something that doesn’t map to any intents in your skill
 * It must also be defined in the language model (if the locale supports it)
 * This handler can be safely added but will be ingnored in locales that do not support it yet 
 * */
const FallbackIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest'
            && Alexa.getIntentName(handlerInput.requestEnvelope) === 'AMAZON.FallbackIntent';
    },
    handle(handlerInput) {
        const speakOutput = 'Sorry, I don\'t know about that. Please try again.';

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};
/* *
 * SessionEndedRequest notifies that a session was ended. This handler will be triggered when a currently open 
 * session is closed for one of the following reasons: 1) The user says "exit" or "quit". 2) The user does not 
 * respond or says something that does not match an intent defined in your voice model. 3) An error occurs 
 * */
const SessionEndedRequestHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'SessionEndedRequest';
    },
    handle(handlerInput) {
        console.log(`~~~~ Session ended: ${JSON.stringify(handlerInput.requestEnvelope)}`);
        // Any cleanup logic goes here.
        return handlerInput.responseBuilder.getResponse(); // notice we send an empty response
    }
};
/* *
 * The intent reflector is used for interaction model testing and debugging.
 * It will simply repeat the intent the user said. You can create custom handlers for your intents 
 * by defining them above, then also adding them to the request handler chain below 
 * */
const IntentReflectorHandler = {
    canHandle(handlerInput) {
        const { request } = handlerInput.requestEnvelope;
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest';
    },
    handle(handlerInput) {
        const intentName = Alexa.getIntentName(handlerInput.requestEnvelope);
        var speakOutput = `Sending ${intentName} command to dataserver` ;
        
        var applianceId = handlerInput.requestEnvelope.request.intent.slots.id.value;  
        
        var options = {
          method: 'GET',
          url: 'https://vama.herokuapp.com/api/view.php',
          params: {id: `${applianceId}`}
        };

        axios.request(options).then(function (response) {
            console.log(response.data);
            
          
            var applianceName = response.data.name;
            var curStatus = response.data.status;
            var curChannel = response.data.channel;
            var curVolume = response.data.volume;

            var changeStatus = curStatus;
            var changeChannel = curChannel;
            var changeVolume = curVolume;
          
            if (intentName === "PowerIntent") {
                var onOff = handlerInput.requestEnvelope.request.intent.slots.onOffCommand.value;
            
                changeStatus = 1;
                speakOutput = `powering appliance ${applianceId} on`;
                if (onOff.toLowerCase() === "off") {
                    changeStatus = 0;
                    speakOutput = `powering appliance ${applianceId} off`;
                }
            
            }   
            else if (intentName === "ChannelVolumeIntent") {
                
                var cv = handlerInput.requestEnvelope.request.intent.slots.channelVolume.value;
                var cc = handlerInput.requestEnvelope.request.intent.slots.changeCommand.value;
                
                if (cc.toLowerCase() === "up") {
                    if (cv.toLowerCase() === "volume") {
                        changeVolume = Number(curVolume) + 1;
                        speakOutput = `appliance ${applianceId} volume up`;
                    }
                    else {
                        changeChannel = Number(curChannel) + 1;
                        speakOutput = `appliance ${applianceId} channel up`;
                    }
                }
                else if (cc.toLowerCase() === "down") {
                    if (cv.toLowerCase() === "volume") {
                        changeVolume = Number(curVolume) - 1;
                        speakOutput = `appliance ${applianceId} volume down`;
                    }
                    else {
                        changeChannel = Number(curChannel) - 1;
                        speakOutput = `appliance ${applianceId} channel down`;
                    }
                }
                else {
                    if (cv.toLowerCase() === "volume") {
                        changeVolume = cc;
                        speakOutput = `appliance ${applianceId} changing volume to ${cc}`;
                    }
                    else {
                        changeChannel = cc;
                        speakOutput = `appliance ${applianceId} changing channel to ${cc}`;
                    }
                }
            }   
            
             var options2 = {
              method: 'POST',
              url: 'https://vama.herokuapp.com/api/update.php',
              headers: {'Content-Type': 'application/json'},
              data: {id: `${applianceId}`, name: `${applianceName}`, status: `${changeStatus}`, channel: `${changeChannel}`, volume: `${changeVolume}`}
            };
            
            axios.request(options2).then(function (response) {
              console.log(response.data);
            }).catch(function (error) {
              console.error(error);
            });
          
        }).catch(function (error) {
          console.error(error);
        });
        

        return handlerInput.responseBuilder.speak(speakOutput).getResponse();
        //tell appliance controller turn appliance one on
    }
};




const PowerIntentHandler = {
    
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest';
    },
    handle(handlerInput) {
        //const intentName = Alexa.getIntentName(handlerInput.requestEnvelope);
        const speakOutput = "";
        //const speakOutput = `toggling ${intentName}`

        return handlerInput.responseBuilder
            .speak(speakOutput)
            //.reprompt('add a reprompt if you want to keep the session open for the user to respond')
            .getResponse();
    }
};

const VolumeIntentHandler = {
    canHandle(handlerInput) {
        return Alexa.getRequestType(handlerInput.requestEnvelope) === 'IntentRequest';
    },
    handle(handlerInput) {
        const intentName = Alexa.getIntentName(handlerInput.requestEnvelope);
        
        const speakOutput = `You just triggered  ${intentName}`;

        return handlerInput.responseBuilder
            .speak(speakOutput)
            //.reprompt('add a reprompt if you want to keep the session open for the user to respond')
            .getResponse();
    }
};







/**
 * Generic error handling to capture any syntax or routing errors. If you receive an error
 * stating the request handler chain is not found, you have not implemented a handler for
 * the intent being invoked or included it in the skill builder below 
 * */
const ErrorHandler = {
    canHandle() {
        return true;
    },
    handle(handlerInput, error) {
        const speakOutput = 'Sorry, I had trouble doing what you asked. Please try again.';
        console.log(`~~~~ Error handled: ${JSON.stringify(error)}`);

        return handlerInput.responseBuilder
            .speak(speakOutput)
            .reprompt(speakOutput)
            .getResponse();
    }
};

/**
 * This handler acts as the entry point for your skill, routing all request and response
 * payloads to the handlers above. Make sure any new handlers or interceptors you've
 * defined are included below. The order matters - they're processed top to bottom 
 * */
exports.handler = Alexa.SkillBuilders.custom()
    .addRequestHandlers(
        LaunchRequestHandler,
        HelloWorldIntentHandler,
        HelpIntentHandler,
        CancelAndStopIntentHandler,
        FallbackIntentHandler,
        SessionEndedRequestHandler,
        IntentReflectorHandler,
        PowerIntentHandler,
        VolumeIntentHandler)
    .addErrorHandlers(
        ErrorHandler)
    .withCustomUserAgent('sample/hello-world/v1.2')
    .lambda();
