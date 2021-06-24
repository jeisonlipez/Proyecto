const { text } = require('express');
const express = require('express');
const { body } = require('express-validator');
const router = express.Router();

const pool = require('../database');
const {isLoggedIn} = require('../lib/auth');

router.get('/procesos', isLoggedIn, async (req, res) => {
    res.send('Su proceso se ha ejecutado con satisfacción');
 });

// Proceso enviado desde el ESP8266 para auditar
router.get('/procesos/:url', isLoggedIn, async (req, res) => {
    const {url} = req.params;
    //console.log(url);
 
    //------------AUDITORIA---------//
   const link = await pool.query('SELECT * FROM links WHERE url = ?', 'http://'+[url]);
   //console.log('Prueba '+link);
   if (link != ''){
      const id = link[0].id;
      const {username,fecha, links, comentario, accion} = ("",null,"","","");
      const newLink = {
         username,
         fecha, 
         links, 
         comentario, 
         accion
      }; 
      newLink.username = 'ESP8266';
      newLink.fecha = new Date();
      //console.log(newLink.fecha);
      newLink.links=id;
      //console.log('Prueba '+id);
      //console.log(link[0]);
      
      if(link[0].estado=="ENCENDIDO"){
         newLink.comentario="Se apagó el dispositivo: "+id; 
      }else{
         newLink.comentario="Se encendió el dispositivo: "+id; 
      }
      newLink.accion=link[0].estado;   
      await pool.query('INSERT INTO auditoria set ?', [newLink]);
      //------------FIN AUDITORIA-----------------//

      //-----------CONSULTAR ESTADO LED-----------//
      console.log(link[0].url+'/state');
      const http = require('http');
      
      http.get(link[0].url+'/state', (resp) => {
      let data = '';
         // Un fragmento de datos ha sido recibido.
         resp.on('data', (chunk) => {
            data += chunk;
         });

         // Toda la respuesta ha sido recibida. Imprimir el resultado.
         resp.on('end', async() => {
            console.log(data);
            if (data == '1'){
               console.log('ENCENDIDO');
               await pool.query('UPDATE links set estado = "APAGADO" WHERE id = ?', [id]);
            }else if (data == '0'){
               console.log('APAGADO');
               await pool.query('UPDATE links set estado = "ENCENDIDO" WHERE id = ?', [id]);
            }
         });
      }).on("error", (err) => {
            console.log("Error LED no conectado: " + err.message);
      });
   }
   //-----------FIN CONSULTAR ESTADO LED-----------//
   
   res.send('Su proceso se ha ejecutado con satisfacción');

 });

 module.exports = router;