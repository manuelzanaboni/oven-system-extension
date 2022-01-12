import { createMuiTheme } from '@material-ui/core/styles';
import { green, red } from '@material-ui/core/colors';

export default createMuiTheme({
palette: {
        primary: {
            main: green[500],
            contrastText: '#FFFFFF'
        },
        secondary: red
    }
});