const express = require('express');
const morgan = require('morgan');
const exphbs = require('express-handlebars');
const path = require('path');
const flash = require('connect-flash');
const session = require('express-session');
const MySQLStore = require('express-mysql-session');
const passport = require('passport');

const { database } = require('./keys');

//inicializaciones (initialization)
const app = express();
require('./lib/passport');

//Configuraciones (settings)
app.set('port', process.env.PORT || 4000);
app.set('views', path.join(__dirname, 'views'));
app.engine('.hbs', exphbs({
    defaultLayout: 'main',
    layoutsDir: path.join(app.get('views'), 'layouts'), 
    partialsDir: path.join(app.get('views'), 'partials'),
    extname: '.hbs',
    helpers: require('./lib/handlebars')
}));
app.set('view engine', '.hbs');

//Programas intermedios (middlewares)
app.use(session({
    secret: 'leduts',
    resave: false,
    saveUninitialized: false,
    store: new MySQLStore(database)
}));
app.use(flash());
app.use(morgan('dev'));
app.use(express.urlencoded({extended: false}));
app.use(express.json());
app.use(passport.initialize());
app.use(passport.session());

//Variables Globales
app.use((req, res, next) => {
    app.locals.success = req.flash('success');
    app.locals.message = req.flash('message');
    app.locals.user = req.user;
    next();
});

//Rutas (routes)
app.use(require('./routes/index.js'));
app.use(require('./routes/autentication.js'));
app.use(require('./routes/procesos.js'));
app.use(require('./routes/auditoria.js'));
app.use('/links', require('./routes/links.js'));

//Archivos publicos (Public)
app.use(express.static(path.join(__dirname, 'public')));

//Iniciar el servidor (starting server)
app.listen(app.get('port'), () => {
    console.log('Servidor en puerto', app.get('port'));
});