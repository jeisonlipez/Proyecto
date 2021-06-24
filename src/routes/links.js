const { text } = require('express');
const express = require('express');
const { body } = require('express-validator');
const router = express.Router();

//const db = require('../database');
const pool = require('../database');
const {isLoggedIn} = require('../lib/auth');

router.get('/add', isLoggedIn, (req, res) => {
   // res.send('Formulario');
   res.render('links/add');
});

//Grabar los resultados de la web a la base de datos
router.post('/add', isLoggedIn, async (req, res) => {
   const { title, url, description, estado} = req.body;
  // console.log(req.body);
   const newLink = {
       title,
       url,
       description,
       user_id: req.user.id,
       estado
   };
   newLink.estado="APAGADO";
   await pool.query('INSERT INTO links set ?', [newLink]);
   //res.send('Recibido');
   req.flash('success', 'Enlace guardado satisfactoriamente');
   res.redirect('/links');
});

//Consultar la base de datos y presentarlos en una vista
router.get('/', isLoggedIn, async (req, res) => {
   const links = await pool.query('SELECT * FROM links WHERE user_id = ?', [req.user.id]);
      res.render('links/list', { links });
      //console.log(links);
      //res.send('listas irán aquí');
});

//Eliminar en la base de datos
router.get('/deleted/:id', isLoggedIn, async (req, res) => {
   //console.log('req.params.id');
   //res.send('ELIMINADO');
   const {id} = req.params;
   await pool.query('DELETE FROM links WHERE ID = ?', [id]);
   req.flash('success', 'Enlace eliminado satisfactoriamente');
   res.redirect('/links');
});

//Editar en la base de datos
router.get('/edit/:id', isLoggedIn, async (req, res) => {
   const {id} = req.params;
   //console.log(id);
   //res.send('RECIBIDO');
   const links = await pool.query('SELECT * FROM links WHERE ID = ?', [id]);
   //console.log(links[0]);
   res.render('links/edit', {link: links[0]});
});

//Actualizar la base de datos con los nuevos datos
router.post('/edit/:id', isLoggedIn, async (req, res) => {
   const {id} = req.params;
   const {title, description, url} = req.body;
   const newLink = {
      title,
      description,
      url
   };
   //console.log(newLink);
   //res.send('ACTUALIZADO');
   await pool.query('UPDATE links set ? WHERE ID = ?', [newLink, id]);
   req.flash('success', 'Enlace actualizado satisfactoriamente');
   res.redirect('/links');
});

//Actualizar boton encender bombillo
router.get('/led/:id', isLoggedIn, async (req, res) => {
   //console.log(req.params.id);
   //console.log(req.user);
   //console.log(req.body);
   //res.send('ACTUALIZADO');
   const {id} = req.params;
   //await pool.query('UPDATE links set estado = (CASE WHEN estado="ENCENDIDO" THEN "APAGADO" ELSE "ENCENDIDO" END) WHERE id = ?', [id]); 
   const link = await pool.query('SELECT * FROM links WHERE ID = ?', [id]);
   
//-----------CONSULTAR ESTADO LED-----------//
console.log(link[0].url+'/state');
const http1 = require('http');

http1.get(link[0].url+'/state', (resp) => {
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
//-----------FIN CONSULTAR ESTADO LED-----------//

   //------------AUDITORIA---------//
   const {username,fecha, links, comentario, accion} = ("",null,"","","");
   const newLink = {
      username,
      fecha, 
      links, 
      comentario, 
      accion
   }; 
   newLink.username = req.user.username;
   newLink.fecha = new Date();
   //console.log(newLink.fecha);
   newLink.links=id;
   //console.log(link[0]);
   if(link[0].estado=="ENCENDIDO"){
      newLink.comentario="Se apagó el dispositivo: "+id; 
   }else{
      newLink.comentario="Se encendió el dispositivo: "+id; 
   }
   newLink.accion=link[0].estado;   
   await pool.query('INSERT INTO auditoria set ?', [newLink]);
   //------------FIN AUDITORIA-----------------//

   //-----------ENVIAR ORDEN DE ENCENDER/APAGAR LED-----------//
   console.log(link[0].url+'/update?state');
   const http = require('http');

   http.get(link[0].url+'/update?state', (resp) => {
   let data = '';

      // Un fragmento de datos ha sido recibido.
      resp.on('data', (chunk) => {
         data += chunk;
      });

      // Toda la respuesta ha sido recibida. Imprimir el resultado.
      resp.on('end', () => {
         console.log(data);
      });

   }).on("error", (err) => {
        console.log("Error LED no conectado: " + err.message);
   });
   //-----------FIN ORDEN DE ENCENDER/APAGAR LED-----------//

   res.redirect('/links');
});

module.exports = router;