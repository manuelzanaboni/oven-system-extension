import { makeStyles } from '@material-ui/core/styles';

export const useStyles = makeStyles((theme) => ({
    main: {
      height: '-webkit-fill-available',
      padding: theme.spacing(3),
      rowGap: theme.spacing(1),
    },
    flexRow: {
      display: 'flex',
      flexDirection: 'row',
    },
    flexColumn: {
      display: 'flex',
      flexDirection: 'column',
    },
    landing: {
      margin: 'auto',
      textAlign: 'center',
      padding: theme.spacing(5)
    },
    lists: {
      columnGap: theme.spacing(2),
      flexWrap: 'wrap'
    },
    list: {
      width: '30%',
      minWidth: 350
    },
    states: {
      width: 'fit-content',
      height: 'fit-content',
      padding: theme.spacing(2),
      marginTop: theme.spacing(1)
    },
    state: {
      columnGap: theme.spacing(2),
      marginTop: theme.spacing(1),
      alignItems: 'center',
      cursor: 'pointer'
    },
    paper: {
      color: 'white',
      width: '70px',
      height: '40px',
      display: 'flex',
      alignItems: 'center',
      justifyContent: 'center'
    },
    on: { 
      backgroundColor: 'green'
    },
    off: { 
      backgroundColor: 'red'
    },
    spacer: {
      flexGrow: 1
    },
    footer: {
      alignItems: 'center',
      columnGap: '25px',
      height: '45px'
    },
    backdrop: {
      zIndex: theme.zIndex.drawer + 1,
      // color: '#fff'
    },
    backdropContainer: {
      // width: "80%",
      // height: "80%",
      width: "fit-content",
      height: "fit-content",
      maxHeight: 700,
      padding: theme.spacing(2),
      overflowY: "scroll"
    },
    ingredients: {
      padding: theme.spacing(1),
      margin: theme.spacing(1),
      alignSelf: "flex-start"
    },
    media: {
      justifyContent: "center"
    },
    backDropTitle: {
      alignItems: "flex-end"
    },
    pizzaImage: {
      borderRadius: 5,
      maxWidth: 200,
      alignSelf: "flex-start",
      marginTop: theme.spacing(1),
      marginRight: "auto",
      marginLeft: "auto",
      marginBottom: theme.spacing(3)
    },
    chart: {
      borderRadius: 5,
      maxWidth: 400,
      alignSelf: "flex-start",
    },
    kcal: { marginLeft: theme.spacing(3) },
    source: { 
      margin: theme.spacing(1),
      alignSelf: "flex-start" 
    }
  }));