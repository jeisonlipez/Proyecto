const { text } = require('express');
const express = require('express');
const { body } = require('express-validator');
const router = express.Router();

const pool = require('../database');
const {isLoggedIn} = require('../lib/auth');

//Consultar la base de datos y presentar Auditoria en una vista
router.get('/auditoria', isLoggedIn, async (req, res) => {
    const auditoria = await pool.query('SELECT * FROM auditoria WHERE id');
       res.render('audi/auditoria', { auditoria });
       //console.log(links);
       //res.send('listas irán aquí');
 });

 module.exports = router;