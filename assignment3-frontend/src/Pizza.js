import React, { useState } from 'react';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';
import Button from '@material-ui/core/Button';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemAvatar from '@material-ui/core/ListItemAvatar';
import Avatar from '@material-ui/core/Avatar';
import ListItemText from '@material-ui/core/ListItemText';
import clsx from  'clsx';
import { useStyles } from './styles';
import Api from './api/api';
const api = new Api();
const DEFAULT_IMAGE = "https://wellness.consumerfreedom.com/app/themes/health-wellness/assets/img/ingredient-placeholder.svg";

export default function Pizza(props) {
  const classes = useStyles();

  const [data, setData] = useState(null);
  const [loaded, setLoaded] = useState(false);

  function getData() {
    setLoaded(true);
    const randomNumber = Math.floor(Math.random() * 100);
    api.get(randomNumber, randomNumber + 1)
    .then(res => {
      setData(res.hits[0].recipe);
    });
  };

  // get data if needed
  if(props.open && !loaded)
    getData();

  // reset loaded state and flush data
  if(!props.open && loaded){
    setLoaded(false);
    setData(null);
  }

  function buildChartString(chartData) {
    let stringData = {
      type: "pie",
      data: {
        labels: [chartData.FAT.label, chartData.CHOCDF.label, chartData.PROCNT.label],
        datasets:[{
          data:[
            Math.floor(chartData.FAT.quantity), 
            Math.floor(chartData.CHOCDF.quantity), 
            Math.floor(chartData.PROCNT.quantity)]
        }]
      },
      options: {
        title: {
          display: true,
          text: 'Nutrients chart [g]',
          fontColor: 'black',
          fontSize:18
        },
        legend: {
          labels: {
            fontColor: 'black',
            fontSize:18
          }
        },
        plugins: {
          datalabels: {
            color: 'black',
            font: {
              size: 18,
              family: "Roboto"
            }
          }
        }
      }
    }
    return `https://quickchart.io/chart?c=${JSON.stringify(stringData)}`;
  }

  function handleClick(url) {
    props.sendTelegramMessage(url);
    window.open(url, "_blank");
  }

  if(data) {
    return(
      <Paper className={clsx(classes.flexColumn, classes.backdropContainer)}> 
        <div className={clsx(classes.flexRow, classes.backDropTitle)}>
          <Typography variant="h5">Proposed recipe: &nbsp;</Typography>
          <Typography variant="h4">{data.label}</Typography>
        </div>

        <div className={classes.flexRow}>
          <Paper elevation={3} className={clsx(classes.flexColumn, classes.ingredients)}>
            <Typography variant="h6">Ingredients:</Typography>
            <List dense={true}>
            {data.ingredients.map((el, index) => {
              return(
                <ListItem key={index}>
                  <ListItemAvatar>
                    <Avatar alt={`ingredient ${el.text}`} src={el.image ? el.image : DEFAULT_IMAGE} />
                  </ListItemAvatar>
                  <ListItemText primary={el.text} secondary={el.foodCategory}/>
                </ListItem>
              );
            })}
            </List>
          </Paper>
          <div className={clsx(classes.flexColumn, classes.media)}>
            <img className={classes.pizzaImage} src={data.image} alt="pizza"/>
            <img className={classes.chart} alt="nutrient chart" src={buildChartString(data.totalNutrients)}></img>
            <Typography className={classes.kcal}variant="body1">KCal: {Math.floor(data.calories)}</Typography>
          </div>
        </div>
      
        { props.TOPIC_TELEGRAM &&
          <Button 
          variant="outlined"
          color="primary"
          className={classes.source}
          onClick={() => handleClick(data.url)}>
            Go to source website
          </Button>}
      </Paper>
    );
  } else return null;
}