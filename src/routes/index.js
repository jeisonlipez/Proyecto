const express = require('express');
const router = express.Router();

router.get('/',(req,res) => {
    //res.send('Hola probando servidor');
    res.render('index');
});

module.exports = router;