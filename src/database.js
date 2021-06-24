const mysql = require ('mysql');
const {promisify} = require('util');

const { database } = require ('./keys');
const { connect } = require('./routes');

const pool = mysql.createPool(database);

pool.getConnection((err, connection) => {
    if (err) {
        if (err.code === 'PROTOCOL_CONNECTION_LOST'){
            cconsole.error ('DATABASE CONNECTION WAS CLOSED');
        }
        if (err.code === 'ER_CON_COUNT_ERROR'){
            console.error ('DATABASE HAS TO MANY CONNECTIONS');
        }
        if (err.code === 'ECONNREFUSED'){
            console.error ('DATABASE CONNECTION WAS REFUSED');
        }
    } 
    
    if (connection) connection.release();
        console.log('Base de datos conectada');
        return;   
});

//Promisify Pool Querys
pool.query = promisify(pool.query);

module.exports = pool;