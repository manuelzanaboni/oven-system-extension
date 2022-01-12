import React, { useState } from 'react';
import clsx from  'clsx';
import Typography from '@material-ui/core/Typography';
import { CircularProgress } from '@material-ui/core';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemIcon from '@material-ui/core/ListItemIcon';
import ListItemText from '@material-ui/core/ListItemText';
import ListItemSecondaryAction from '@material-ui/core/ListItemSecondaryAction';
import Slider from '@material-ui/core/Slider';
import Paper from '@material-ui/core/Paper';
import { Icon } from "@material-ui/core";
import Button from '@material-ui/core/Button';
import Thermometer from './thermometer.svg';
import SpeedIcon from '@material-ui/icons/Speed';
import SignalWifi1BarIcon from '@material-ui/icons/SignalWifi1Bar';
import Backdrop from '@material-ui/core/Backdrop';
import mqtt from 'mqtt';
import theme from './theme'
import { ThemeProvider } from '@material-ui/core/styles';

import { useStyles } from './styles'
import Pizza from './Pizza';
import Settings from './Settings';

// mqtt client
let client;
const TOPIC_CONFIG_ASK = 'oven/config';
const TOPIC_CONFIG_RECEIVE = 'oven/config/frontend';
// filled at runtime
let TOPIC_SENSORS = null;
let TOPIC_STATE = null;
let TOPIC_CONTROL = null;
let TOPIC_TELEGRAM = null;

function App() {
  const classes = useStyles();

  const [connect, setConnect] = useState(true);
  const [temps, setTemps] = useState([]);
  const [press, setPress] = useState([]);
  const [state, setState] = useState({setPoint: 100});
  const [burnerDisabled, setBurnerDisabled] = useState(false);
  const [resistanceDisabled, setResistanceDisabled] = useState(false);
  const [loading, setLoading] = useState(false);
  const [backdropOpen, setBackdropOpen] = useState(false);

  const tempLabels = [
    'Sonda forno', 'Sonda platea', 'Sonda puffer', 'Sonda fumi'
  ]
  const pressLabels = [
    'Delta pressione forno', 'Delta pressione gas'
  ]

  function connectToBroker() {
    setLoading(true);
    setConnect(false);
    const options = {
      will: {
        topic: TOPIC_CONFIG_ASK,
        payload: 'frontend-bye',
        qos: 2,
        retain: false,
        resubscribe: false
      }
    }
    client = mqtt.connect('ws://192.168.1.254:9001', options);
    client.on('connect', () => {
      console.log("CONNECTED");
      client.subscribe(TOPIC_CONFIG_RECEIVE, {qos: 2}, (err, granted) =>{
        client.publish(TOPIC_CONFIG_ASK, 'frontend', {qos: 2});
      });
    });
    client.on('message', (topic, payload, packet) => handleMessage(topic, payload, packet)); 
  }

  function handleMessage(topic, payload, packet) {
    // console.log(packet)
    if(topic && topic === TOPIC_CONFIG_RECEIVE)
      configTopics(payload);

    if(topic && topic === TOPIC_SENSORS) {
      const data = JSON.parse(payload.toString());
      console.log(data)
      setTemps(Object.values(data["temps"]));
      setPress(Object.values(data["press"]));
      setLoading(false);
    }
    if(topic && topic === TOPIC_STATE) {
      const data = JSON.parse(payload.toString());
      setState(data);
      setBurnerDisabled(false);
      setResistanceDisabled(false);
    }
  }

  function configTopics(payload) {
    const topics = JSON.parse(payload);
    console.log(topics)
    if(topics.sensors) {
      TOPIC_SENSORS = topics.sensors;
      client.subscribe(TOPIC_SENSORS, {qos: 0});
    }
    if(topics.state) {
      TOPIC_STATE = topics.state;
      client.subscribe(TOPIC_STATE, {qos: 1});
    }
    if(topics.control)
      TOPIC_CONTROL = topics.control;
    if(topics.telegram)
      TOPIC_TELEGRAM= topics.telegram;
  }

  // display temperatures list
  function displayTemps() {
    return(
      <div className={classes.list}>
        <List component="nav">
          {temps?.map((temp, index) => {
            return (
              <Paper elevation={3} key={index} style={{marginBottom: '10px'}}>
                <ListItem>
                  <ListItemIcon>
                    <Icon fontSize="large">
                      <img src={Thermometer} height={30} width={30} alt="thermometer icon"/>
                    </Icon>
                  </ListItemIcon>
                  <ListItemText primary={tempLabels[index]} primaryTypographyProps={{variant: "h6"}}/>
                  <ListItemSecondaryAction>
                    <Typography variant="h6">{temp} °C</Typography>
                  </ListItemSecondaryAction>
                </ListItem>
              </Paper>
            )
          })}
        </List>
      </div>
    );
  }

  // display pressures list
  function displayPress() {
    return(
      <div className={classes.list}>
        <List component="nav">
          {press?.map((pres, index) => {
            return (
              <Paper elevation={3} key={index} style={{marginBottom: '10px'}}>
                <ListItem>
                    <ListItemIcon>
                        <SpeedIcon style={{ fontSize: 30 }} color="primary"/>
                    </ListItemIcon>
                    <ListItemText primary={pressLabels[index]} primaryTypographyProps={{variant: "h6"}}/>
                    <ListItemSecondaryAction >
                    <Typography variant="h6">{pres} Pa</Typography>
                    </ListItemSecondaryAction>
                </ListItem>
              </Paper>
            )
            })}
        </List>
      </div>
    );
  }

  // display ON/OFF state
  function displayState(state, type) {
    if(!state)
      return(
        <ThemeProvider theme={theme}>
          <Button 
          variant="contained" size="large" color="secondary" 
          disabled={type === "burner" ? burnerDisabled : resistanceDisabled}
          onClick={() => {handleButtons(state, type)}}>
            OFF
          </Button>
        </ThemeProvider>
      );
    else
      return(
        <ThemeProvider theme={theme}>
          <Button 
          variant="contained" size="large" color="primary"
          disabled={type === "burner" ? burnerDisabled : resistanceDisabled}
          onClick={() => {handleButtons(state, type)}}>
            ON
          </Button>
        </ThemeProvider>
      );
  }

  function handleButtons(state, type) {
    if(TOPIC_CONTROL) {
      client.publish(TOPIC_CONTROL, `{"${type}": ${!state}}`, {qos: 1});
      type === "burner" ? setBurnerDisabled(true) : setResistanceDisabled(true);
    }
  }

  function sliderChange(event, value) {
    setState({setPoint: value});
    if(TOPIC_CONTROL)
      client.publish(TOPIC_CONTROL, `{"thermostat": ${value}}`, {qos: 1});
  }

  // wifi low icon
  // eslint-disable-next-line
  function displayWifiLow () {
    return <SignalWifi1BarIcon style={{ color: "red" }}/>;
  }

  function landing() {
    return(
      <>
      <Paper elevation={3} className={classes.landing}>
        <Typography variant="h4">Benvenuto!<br />Clicca sul bottone per connetterti</Typography>
        <br />
        <Button variant="outlined" color="primary" onClick={connectToBroker}>
          Connetti
        </Button>
     </Paper>
     {pizza()}
     </>
    );
  }

  function content() {
    return(
      <>
        {/* temps and pressures */}
        <div className={clsx(classes.lists, classes.flexRow)}>
          {displayTemps()}
          {displayPress()}
  
        {temps[0] ?
          <Paper elevation={3} className={classes.states}>
            {/* thermostat */}
            <Typography variant="h6">SetPoint termostato: {state.setPoint} °C</Typography> 
            <Slider style={{width: 300}}
            value={state.setPoint}
            onChange={sliderChange}
            aria-labelledby="discrete-slider"
            valueLabelDisplay="auto"
            step={10} min={100} max={550}/>
    
            {/* states */}
            <div className={clsx(classes.state, classes.flexRow)}>
              <Typography variant="h6">Stato bruciatore:</Typography>
              {displayState(state.burner, "burner")}
            </div>
    
            <div className={clsx(classes.state, classes.flexRow)}>
              <Typography variant="h6">Stato resistenza:</Typography>
              {displayState(state.resistance, "resistance")}
            </div>
          </Paper> : null}
        </div>
  
        <div className={classes.spacer}></div>
  
        {/* footer */}
        <footer className={clsx(classes.footer, classes.flexRow)}>
  
          {state.wifi < -60 ? displayWifiLow() : null}

          {pizza()}
          
          <div className={classes.spacer}></div>
  
          {loading ? 
          <div className={clsx(classes.footer, classes.flexRow)}>
            <Typography variant="body2">Caricamento dati...</Typography>
            <CircularProgress color="primary"/>
          </div>
          : null}
        </footer>
      </>
    );
  }

  const handleBackdropClose = () => {
    setBackdropOpen(false);
  };
  const handleBackdropToggle = () => {
    setBackdropOpen(!backdropOpen);
  };

  const sendTelegramMessage = (url) => {
    console.log(url);
    client.publish(TOPIC_TELEGRAM, `{"url": "${url}"}`, {qos: 0});
  }

  function pizza() {
    return(
      <div>
        <Button variant="outlined" color="primary" onClick={handleBackdropToggle}>
          Consigliami una pizza
        </Button>
        <Backdrop className={classes.backdrop} open={backdropOpen} onClick={handleBackdropClose}>
          <Pizza 
          open={backdropOpen}
          sendTelegramMessage={sendTelegramMessage}
          TOPIC_TELEGRAM={TOPIC_TELEGRAM}/>
        </Backdrop>
      </div>
    );
  }

  function settings() {
    return <Settings />;
  }

  // main return
  return (
    <div className={clsx(classes.main, classes.flexColumn)}>
      {/* title */}
      <Typography variant="h1">Monitoraggio forno</Typography>
      {connect ? landing() : content()}
      {settings()}
    </div>
  );
}
export default App;
