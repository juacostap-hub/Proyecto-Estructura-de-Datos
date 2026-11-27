#ifndef PAGINA_H          // Evita que este archivo se lea más de una vez al compilar
#define PAGINA_H          // Marca que este archivo ya fue leído

// PÁGINA DEL JUEGO (HTML + CSS + JavaScript) guardada como texto de C++.
// El servidor (Servidor.cpp) se la entrega al navegador y pega las imágenes de Jonatan (Sprites.h).
// La página solo DIBUJA: cada tecla o clic llama al servidor y toda la lógica pasa en C++ (Juego.cpp).
// Todo funciona sin internet: no hay fuentes ni imágenes de otras páginas. Los dibujos son SVG y canvas hechos a mano.
// Estilo caricatura: colores de OrderStack (naranja #FCB14E, rojo #DC0E20, crema #F1EAD1) con contorno grueso #2B0A0D.
//
// Partes del JavaScript (en este orden):
//   1. Base: comunicación con C++, encabezado, despacho de pantallas y escenas
//   2. Dibujos: comida, logo, mesero, clientes, cafetería y moto en SVG
//   3. Mapa del restaurante (canvas)
//   4. Día: paneles, reloj con el sol, avisos de cierre y "si el día cierra ahora pierdes"
//   5. Escenas: inicio de jornada, cierre del día y fin de la noche
//   6. Noche: mapa de la ciudad, ruta trazada por el jugador, GPS, viaje, calle cerrada y pedido urgente
//   7. Tienda y reseña final con estrellas
//   8. Pantalla de inicio, teclado y arranque

static const char* PAGINA_HTML = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>OrderStack · Restaurant Rush</title>
<style>
/* Colores de OrderStack + tinta oscura para los contornos gruesos de caricatura */
:root{
  --naranja:#FCB14E; --naranjaOsc:#E8932B; --rojo:#DC0E20; --rojoOsc:#A30A18; --crema:#F1EAD1; --papel:#FFF8E7;
  --tinta:#2B0A0D; --tinta2:#6E2A2E; --tinta3:#9A7466; --amarillo:#FFD23F; --amarilloOsc:#E0A800;
  --verde:#4E9A2F; --verdeBg:#DDEFC9; --rojoBg:#FAD0D2; --azul:#2F7DD1; --noche:#1E2340; --noche2:#2C3263;
  --sombra:4px 5px 0 var(--tinta);
  --titulo:Impact,Haettenschweiler,'Arial Narrow Bold','Arial Black',sans-serif;
  --texto:'Trebuchet MS','Segoe UI',system-ui,-apple-system,Roboto,Arial,sans-serif;
}
*{box-sizing:border-box}
html,body{margin:0}
body{color:var(--tinta);font:15px/1.45 var(--texto);min-height:100vh;display:flex;flex-direction:column;
  background:radial-gradient(circle at 22px 22px,rgba(255,255,255,.22) 5px,transparent 6px) 0 0/44px 44px,var(--naranja);transition:background-color .8s}
body.de-noche{background:radial-gradient(circle at 22px 22px,rgba(255,255,255,.05) 2px,transparent 3px) 0 0/44px 44px,var(--noche)}
button{font:inherit;cursor:pointer;color:inherit}
button:disabled{cursor:not-allowed;opacity:.45}
:focus-visible{outline:3px solid var(--azul);outline-offset:2px}
.oculto{display:none!important}

/* ---------- Encabezado ---------- */
header{background:var(--rojo);color:var(--papel);border-bottom:3px solid var(--tinta);position:sticky;top:0;z-index:20}
body.en-inicio header,body.en-inicio .cuadros{display:none}
.hd{max-width:1500px;margin:0 auto;display:flex;align-items:center;gap:16px;padding:8px 22px;flex-wrap:wrap}
.cuadros{height:16px;background:repeating-conic-gradient(var(--rojo) 0 25%,var(--crema) 0 50%) 0 0/16px 16px;border-bottom:3px solid var(--tinta)}
.logo{display:flex;align-items:center;gap:10px}
.logo svg{width:46px;height:46px;flex:none}
.logo b{font-family:var(--titulo);font-weight:400;font-size:28px;letter-spacing:1px;display:block;line-height:1;-webkit-text-stroke:1px var(--tinta);paint-order:stroke fill;text-shadow:2px 3px 0 var(--tinta)}
.logo small{color:#FFE1A8;font-size:12.5px;font-weight:700}
.kpis{margin-left:auto;display:flex;gap:10px;flex-wrap:wrap;align-items:center}
.kpi{background:var(--papel);color:var(--tinta2);border:2.5px solid var(--tinta);border-radius:12px;padding:4px 12px;font-size:11.5px;line-height:1.2;min-width:100px;box-shadow:2px 3px 0 var(--tinta)}
.kpi b{display:block;font-family:var(--titulo);font-weight:400;letter-spacing:.5px;font-size:20px;color:var(--rojo)}
.barra{height:7px;border-radius:4px;background:#E5D6AE;margin-top:3px;overflow:hidden;border:1.5px solid var(--tinta)}
.barra i{display:block;height:100%;transition:width .3s}
.hbtn{border:2.5px solid var(--tinta);background:var(--papel);color:var(--tinta);border-radius:12px;padding:6px 11px;font-weight:800;box-shadow:2px 3px 0 var(--tinta)}
.hbtn:active{transform:translateY(2px);box-shadow:0 1px 0 var(--tinta)}

/* ---------- Bloques comunes ---------- */
main{max-width:1500px;width:100%;margin:0 auto;padding:18px 22px 34px;flex:1}
body.en-inicio main{max-width:none;padding:0}
.pantalla{display:none}
.pantalla.on{display:block}
.card{background:var(--papel);border:3px solid var(--tinta);border-radius:18px;padding:13px 15px;box-shadow:var(--sombra)}
.card+.card{margin-top:16px}
.card h2{margin:0 0 8px;font-family:var(--titulo);font-weight:400;font-size:21px;letter-spacing:.6px;color:var(--rojo);display:flex;align-items:center;gap:8px;line-height:1.1}
.card h2 small{margin-left:auto;font-family:var(--texto);letter-spacing:0;font-size:12px;color:var(--tinta2);font-weight:700}
.sub{color:var(--tinta2);font-size:13px;margin:0 0 10px}
.vacio{color:var(--tinta3);font-size:13px;font-style:italic;padding:6px 0}
.btn{border:3px solid var(--tinta);border-radius:14px;padding:9px 16px;font-weight:800;display:inline-flex;align-items:center;justify-content:center;gap:8px;box-shadow:0 5px 0 var(--tinta);transition:transform .07s,box-shadow .07s,filter .15s;position:relative;top:0}
.btn:hover{filter:brightness(1.06)}
.btn:active{transform:translateY(3px);box-shadow:0 2px 0 var(--tinta)}
.btn.am{background:var(--amarillo)}
.btn.ro{background:var(--rojo);color:var(--papel)}
.btn.cr{background:var(--crema)}
.btn.ve{background:#7BC453}
.btn.chico{padding:6px 10px;font-size:13px;border-width:2.5px;box-shadow:0 3px 0 var(--tinta)}
.btn.grande{font-family:var(--titulo);font-weight:400;font-size:26px;letter-spacing:1px;padding:10px 28px}
.chip{display:inline-block;font-size:11.5px;font-weight:800;padding:2px 9px;border-radius:999px;background:#FFE7B0;color:var(--tinta);border:1.5px solid var(--tinta)}
.chip.ve{background:var(--verdeBg)} .chip.ro{background:var(--rojoBg)} .chip.az{background:#D7E8FB} .chip.am{background:var(--amarillo)}
kbd{background:var(--papel);border:2px solid var(--tinta2);border-bottom-width:4px;border-radius:6px;padding:0 6px;font:700 12px Consolas,ui-monospace,monospace}
.ico{width:22px;height:22px;flex:none;vertical-align:middle}
.ico.g{width:30px;height:30px}
table.t{width:100%;border-collapse:collapse;font-size:13.5px}
table.t th{text-align:left;color:var(--tinta2);font-size:12px;border-bottom:2.5px solid var(--tinta);padding:6px}
table.t td{padding:6px;border-bottom:1.5px dashed #DCC79B}
table.t tr.yo td{background:var(--amarillo);font-weight:800}

/* ---------- Jonatan en las esquinas ---------- */
.jonatan{display:flex;gap:12px;align-items:flex-start}
.retrato{width:92px;height:92px;flex:none;border-radius:50%;border:3px solid var(--tinta);background:radial-gradient(circle at 50% 35%,#FFE1A8,var(--naranja));overflow:hidden;box-shadow:2px 3px 0 var(--tinta)}
.retrato img{width:100%;height:100%;object-fit:cover;object-position:50% 12%}
.burbuja{background:#fff;border:3px solid var(--tinta);border-radius:16px;padding:9px 12px;font-weight:800;font-size:14px;position:relative;flex:1}
.burbuja:before{content:"";position:absolute;left:-13px;top:20px;border:10px solid transparent;border-right-color:var(--tinta);border-left:0}
.burbuja:after{content:"";position:absolute;left:-8px;top:23px;border:7px solid transparent;border-right-color:#fff;border-left:0}
.burbuja.nueva{animation:sacudir .5s}
@keyframes sacudir{0%,100%{transform:none}20%{transform:translateX(-4px) rotate(-1deg)}40%{transform:translateX(4px) rotate(1deg)}60%{transform:translateX(-3px)}80%{transform:translateX(2px)}}
.amenaza{margin-top:12px}
.amenaza .linea{height:16px;border-radius:8px;background:#3B0F12;overflow:hidden;border:2.5px solid var(--tinta)}
.amenaza .linea i{display:block;height:100%;background:repeating-linear-gradient(45deg,var(--rojo) 0 8px,#FF4655 8px 16px);transition:width .3s}
.amenaza .lbl{display:flex;justify-content:space-between;font-size:12px;font-weight:800;margin-bottom:4px}

/* ---------- Día ---------- */
.dia{display:grid;grid-template-columns:285px minmax(0,1fr) 320px;gap:18px;align-items:start}
.lienzo{background:var(--tinta);border-radius:20px;padding:8px;border:3px solid var(--tinta);box-shadow:var(--sombra);position:relative}
#mapa{display:block;width:100%;height:auto;border-radius:13px;cursor:pointer}
.cartel{position:absolute;left:50%;top:42%;transform:translate(-50%,-50%) scale(.6) rotate(-3deg);opacity:0;pointer-events:none;text-align:center;z-index:5;transition:transform .35s cubic-bezier(.2,1.6,.4,1),opacity .25s}
.cartel.on{opacity:1;transform:translate(-50%,-50%) scale(1) rotate(-3deg)}
.cartel b{display:block;font-family:var(--titulo);font-weight:400;font-size:64px;line-height:1;letter-spacing:2px;color:var(--amarillo);-webkit-text-stroke:3px var(--tinta);paint-order:stroke fill;text-shadow:5px 6px 0 var(--rojoOsc),5px 6px 0 var(--tinta)}
.cartel span{display:inline-block;margin-top:8px;background:var(--papel);border:3px solid var(--tinta);border-radius:12px;padding:6px 14px;font-weight:800;font-size:16px;box-shadow:3px 4px 0 var(--tinta)}
.cartel.peligro b{color:#FF4655}
.reloj{display:flex;align-items:center;gap:12px;margin-top:12px}
.reloj svg{flex:1;height:64px;border:3px solid var(--tinta);border-radius:14px;box-shadow:3px 4px 0 var(--tinta);display:block;min-width:0}
.reloj .lado{display:flex;flex-direction:column;gap:8px;align-items:stretch}
.tiempo{font-family:var(--titulo);font-size:24px;color:var(--rojo);line-height:1;text-align:center;background:var(--papel);border:3px solid var(--tinta);border-radius:12px;padding:6px 10px;box-shadow:3px 4px 0 var(--tinta);min-width:118px}
.tiempo small{display:block;font-family:var(--texto);font-size:11.5px;color:var(--tinta2);font-weight:800;margin-top:2px}
.tiempo.alerta{animation:latir .9s infinite}
@keyframes latir{50%{transform:scale(1.06);background:#FFE0E2}}
.perdidas{margin-top:12px;border:3px solid var(--tinta);border-radius:14px;background:#FFF1D6;padding:9px 13px;font-size:13.5px;box-shadow:3px 4px 0 var(--tinta)}
.perdidas.ok{background:var(--verdeBg)}
.perdidas b.t{font-family:var(--titulo);font-weight:400;font-size:18px;color:var(--rojo);letter-spacing:.5px;display:block;margin-bottom:2px}
.perdidas ul{margin:4px 0 0;padding-left:18px}
.perdidas li{margin:2px 0}
.estado{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:12px}
.estado div{background:var(--papel);border:2.5px solid var(--tinta);border-radius:12px;padding:7px 10px;font-size:13px;display:flex;gap:8px;align-items:center}
.estado b{display:block;font-family:var(--titulo);font-weight:400;font-size:15px;color:var(--rojo);letter-spacing:.4px}
.aviso{margin-top:12px;background:var(--tinta);color:var(--crema);border-radius:12px;padding:9px 14px;font-weight:700;min-height:40px;font-size:14px;border:3px solid var(--tinta)}
.aviso.mal{background:var(--rojoOsc)}
.teclas{font-size:12.5px;margin-top:10px;display:flex;flex-wrap:wrap;gap:6px 14px;font-weight:700}
.grupo{display:flex;align-items:center;gap:10px;padding:7px 9px;border-radius:13px;background:var(--crema);margin-bottom:8px;border:2.5px solid transparent}
.grupo.sig{border-color:var(--tinta);background:#FFF1D6;box-shadow:2px 3px 0 var(--tinta)}
.grupo .caras{display:flex;min-width:58px}
.grupo .caras svg{width:24px;height:24px;margin-right:-8px}
.grupo .info{flex:1;font-size:13px;line-height:1.25}
.grupo .info b{display:block;font-size:13.5px}
.tipo{font-size:10.5px;font-weight:800;padding:1px 7px;border-radius:999px;color:#fff;display:inline-block;border:1.5px solid var(--tinta)}
.pac{height:7px;border-radius:4px;background:#E5D6AE;margin-top:4px;overflow:hidden;border:1.5px solid var(--tinta)}
.pac i{display:block;height:100%}
.pedido{display:flex;align-items:center;gap:10px;padding:6px 10px;border-radius:12px;background:var(--crema);margin-bottom:6px;font-size:13px;border:2.5px solid transparent}
.pedido.raiz{background:#FFF1D6;border-color:var(--tinta);box-shadow:2px 3px 0 var(--tinta)}
.pedido .info{flex:1}
.pedido .t{font-family:var(--titulo);font-size:20px;color:var(--rojo)}
.cuenta{display:flex;justify-content:space-between;font-size:13px;padding:4px 0;border-bottom:1.5px dashed #DCC79B}
.total{display:flex;justify-content:space-between;font-weight:800;margin-top:6px}
.eventos{font-size:12.5px;max-height:190px;overflow:auto}
.eventos div{padding:4px 0;border-bottom:1.5px dashed #DCC79B}
.eventos div:first-child{font-weight:800}

/* ---------- Escenas a pantalla completa (transiciones, ataques) ---------- */
.escena{position:fixed;inset:0;z-index:60;display:flex;align-items:center;justify-content:center;padding:18px;background:rgba(43,10,13,.55);opacity:0;pointer-events:none;transition:opacity .35s}
.escena.on{opacity:1;pointer-events:auto}
.escena .card{max-width:620px;width:100%;max-height:92vh;overflow:auto;transform:translateY(24px) scale(.96);transition:transform .45s cubic-bezier(.2,1.4,.4,1)}
.escena.on .card{transform:none}
.escena .card h2{font-size:30px}
.cielo-transicion{position:fixed;inset:0;z-index:59;pointer-events:none;opacity:0;transition:opacity .5s}
.cielo-transicion.on{opacity:1}
.cifras{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px;margin:12px 0}
.cifras div{background:var(--crema);border:2.5px solid var(--tinta);border-radius:12px;padding:8px 10px;text-align:center;font-size:12px;color:var(--tinta2);font-weight:700}
.cifras b{display:block;font-family:var(--titulo);font-weight:400;font-size:26px;color:var(--rojo);line-height:1.1}
.cifras .mal b{color:var(--rojoOsc)} .cifras .bien b{color:var(--verde)}
.acciones{display:flex;gap:10px;justify-content:flex-end;flex-wrap:wrap;margin-top:12px}
.cutin{position:fixed;inset:0;z-index:70;pointer-events:none;overflow:hidden;opacity:0;transition:opacity .2s}
.cutin.on{opacity:1}
.cutin .franja{position:absolute;left:-10%;right:-10%;top:30%;height:40%;background:repeating-linear-gradient(-12deg,var(--rojo) 0 34px,var(--rojoOsc) 34px 68px);border-top:5px solid var(--tinta);border-bottom:5px solid var(--tinta);transform:skewY(-6deg) translateX(-110%);transition:transform .35s cubic-bezier(.2,1.2,.4,1)}
.cutin.on .franja{transform:skewY(-6deg) translateX(0)}
.cutin img{position:absolute;right:4%;bottom:-6%;height:96%;transform:translateX(120%);transition:transform .4s .1s cubic-bezier(.2,1.3,.4,1);filter:drop-shadow(0 0 0 #fff) drop-shadow(4px 0 0 #fff) drop-shadow(-4px 0 0 #fff) drop-shadow(0 4px 0 #fff) drop-shadow(8px 10px 0 rgba(43,10,13,.6))}
.cutin.on img{transform:none}
.cutin .txt{position:absolute;left:6%;top:36%;width:56%;transform:rotate(-6deg)}
.cutin .txt b{display:block;font-family:var(--titulo);font-weight:400;font-size:clamp(48px,8vw,110px);line-height:.95;color:var(--amarillo);-webkit-text-stroke:4px var(--tinta);paint-order:stroke fill;text-shadow:6px 7px 0 var(--tinta)}
.cutin .txt span{display:inline-block;margin-top:10px;background:var(--papel);border:3px solid var(--tinta);border-radius:12px;padding:6px 14px;font-weight:800;font-size:18px}

/* ---------- Noche ---------- */
.noche{display:grid;grid-template-columns:minmax(0,1fr) 390px;gap:18px;align-items:start}
.noche>div:first-child{position:sticky;top:96px}
.ciudad{background:var(--noche);border-radius:20px;padding:8px;border:3px solid var(--tinta);box-shadow:4px 5px 0 #0D0F22;position:relative}
.ciudad svg{display:block;width:100%;height:auto;border-radius:13px}
.ciudad .nodo{cursor:default}
.ciudad .nodo.clic{cursor:pointer}
.ciudad .anillo{animation:anillo 1s ease-in-out infinite;transform-box:fill-box;transform-origin:center}
@keyframes anillo{50%{transform:scale(1.18);opacity:.55}}
.ciudad .titila{animation:titila 2.4s infinite}
@keyframes titila{50%{opacity:.25}}
body.de-noche .reloj .tiempo{color:var(--noche2)}
.dom{display:flex;gap:10px;align-items:center;padding:8px 10px;border-radius:13px;background:var(--crema);margin-bottom:8px;border:2.5px solid transparent;cursor:pointer}
.dom.sel{border-color:var(--tinta);background:#FFF1D6;box-shadow:2px 3px 0 var(--tinta)}
.dom.urg{background:#FFE3E5}
.dom.hecho{opacity:.62;cursor:default}
.dom .num{font-family:var(--titulo);font-size:22px;color:var(--papel);background:var(--rojo);border:2.5px solid var(--tinta);border-radius:50%;width:36px;height:36px;display:grid;place-items:center;flex:none}
.dom .info{flex:1;font-size:13px;line-height:1.3}
.termo{display:flex;align-items:center;gap:6px;margin-top:3px;font-size:12px;font-weight:800}
.termo .tubo{flex:1;height:9px;border:1.5px solid var(--tinta);border-radius:6px;background:#D7E8FB;overflow:hidden}
.termo .tubo i{display:block;height:100%;border-radius:4px}
.pasos{display:flex;flex-wrap:wrap;gap:4px;margin:6px 0}
.pasos span{background:var(--crema);border:2px solid var(--tinta);border-radius:999px;padding:1px 8px;font-size:12px;font-weight:800}
.pasos span.casa{background:var(--amarillo)}
.pasos i{font-style:normal;font-weight:800;color:var(--tinta3)}
.comparar{display:flex;gap:10px;margin:8px 0}
.comparar div{flex:1;background:var(--crema);border:2.5px solid var(--tinta);border-radius:12px;padding:6px 8px;text-align:center;font-size:12px;font-weight:700;color:var(--tinta2)}
.comparar b{display:block;font-family:var(--titulo);font-weight:400;font-size:24px;color:var(--tinta);line-height:1.1}
.comparar .igual{background:var(--amarillo)}
.botones{display:flex;gap:8px;flex-wrap:wrap;margin-top:8px}
.desvio{background:var(--rojo);color:var(--papel);border:3px solid var(--tinta);border-radius:14px;padding:9px 13px;font-weight:800;margin-bottom:12px;box-shadow:3px 4px 0 var(--tinta);animation:sacudir .6s}
.oferta{border-color:var(--rojo)}
.brindis{position:fixed;left:50%;bottom:26px;transform:translate(-50%,220%);visibility:hidden;z-index:65;background:var(--papel);border:3px solid var(--tinta);border-radius:16px;padding:10px 18px;font-weight:800;box-shadow:var(--sombra);transition:transform .4s cubic-bezier(.2,1.4,.4,1);max-width:90vw;text-align:center}
.brindis.on{transform:translate(-50%,0);visibility:visible}
.brindis.oro{background:var(--amarillo)}

/* ---------- Tienda ---------- */
.tienda{display:grid;grid-template-columns:minmax(0,1fr) 380px;gap:18px;align-items:start}
.mejoras{display:grid;grid-template-columns:repeat(2,1fr);gap:14px}
.mejora{background:var(--crema);border-radius:16px;padding:14px;border:3px solid var(--tinta);display:flex;flex-direction:column;gap:8px;box-shadow:3px 4px 0 var(--tinta)}
.mejora.tiene{background:var(--verdeBg)}
.mejora svg{width:56px;height:56px}
.mejora b{font-family:var(--titulo);font-weight:400;font-size:22px;color:var(--rojo);letter-spacing:.5px}
.mejora p{margin:0;font-size:13px;flex:1}
.mejora .chip{align-self:flex-start}

/* ---------- Final: la reseña ---------- */
.final{display:grid;grid-template-columns:minmax(0,1.25fr) minmax(0,1fr);gap:22px;align-items:start}
.periodico{background:#FBF4E2;border:3px solid var(--tinta);border-radius:6px;padding:20px 24px;box-shadow:6px 7px 0 var(--tinta);transform:rotate(-1.2deg);font-family:Georgia,'Times New Roman',serif;position:relative}
.periodico .cab{text-align:center;border-bottom:3px double var(--tinta);padding-bottom:6px;margin-bottom:10px}
.periodico .cab b{font-family:Georgia,serif;font-size:30px;letter-spacing:.5px;display:block}
.periodico .cab small{font-size:12px;color:var(--tinta2)}
.periodico h3{font-size:25px;margin:6px 0 2px;line-height:1.15}
.estrellas{display:flex;gap:8px;margin:10px 0 6px}
.estrellas svg{width:54px;height:54px;transform:scale(0) rotate(-40deg);transition:transform .35s cubic-bezier(.2,1.8,.4,1)}
.estrellas svg.on{transform:none}
.periodico p{margin:8px 0;font-size:16px;line-height:1.55}
.periodico .firma{text-align:right;font-style:italic}
.sello{position:absolute;right:26px;bottom:44px;border:4px solid var(--rojo);color:var(--rojo);font-family:var(--titulo);font-size:26px;padding:2px 12px;border-radius:8px;transform:rotate(-8deg) scale(2);opacity:0;transition:all .3s cubic-bezier(.2,1.6,.4,1);letter-spacing:1px;background:rgba(255,255,255,.4)}
.sello.on{opacity:.9;transform:rotate(-8deg) scale(1)}
.sello.verde{border-color:var(--verde);color:var(--verde)}
.jefe{display:flex;flex-direction:column;align-items:center;gap:10px}
.jefe img{width:300px;max-width:80%;height:auto;filter:drop-shadow(6px 7px 0 rgba(43,10,13,.35))}
.jefe .burbuja{max-width:340px;font-size:16px;opacity:0;transform:scale(.7);transition:all .35s cubic-bezier(.2,1.6,.4,1)}
.jefe .burbuja.on{opacity:1;transform:none}
.jefe .burbuja:before,.jefe .burbuja:after{display:none}
.resultado{font-family:var(--titulo);font-weight:400;font-size:52px;line-height:1;margin:0;color:var(--amarillo);-webkit-text-stroke:3px var(--tinta);paint-order:stroke fill;text-shadow:4px 5px 0 var(--tinta)}

/* ---------- Modales ---------- */
.modal{position:fixed;inset:0;background:rgba(43,10,13,.6);display:none;align-items:center;justify-content:center;z-index:80;padding:16px}
.modal.on{display:flex}
.modal .card{max-width:660px;width:100%;max-height:90vh;overflow:auto}
.modal ul{margin:6px 0 12px;padding-left:20px}
.modal li{margin:3px 0}
.campo{display:flex;gap:10px;flex-wrap:wrap;margin-top:8px}
.campo input{flex:1;min-width:180px;border:3px solid var(--tinta);border-radius:12px;padding:10px 14px;font:inherit;font-size:17px;background:#fff}

@media(max-width:1250px){.noche>div:first-child{position:static}.dia{grid-template-columns:1fr 1fr}.dia .centro{grid-column:1/-1;order:-1}.noche,.tienda,.final{grid-template-columns:1fr}}
@media(max-width:760px){.dia{grid-template-columns:1fr}main{padding:12px}.estado{grid-template-columns:1fr}.mejoras{grid-template-columns:1fr}.cartel b{font-size:44px}}
@media(prefers-reduced-motion:reduce){*,*:before,*:after{animation-duration:.01s!important;animation-iteration-count:1!important;transition-duration:.01s!important}}

/* ---------- Cielo de la transición día → noche ---------- */
.cielo-transicion .ct-capa{position:absolute;inset:0;transition:opacity 1.7s}
.ct-tarde{background:linear-gradient(#FF9E5E,#F7C46C 55%,#FCB14E)}
.ct-noche{background:linear-gradient(#141833,#2C3263 70%,#4A3F86);opacity:0}
.cielo-transicion.anochece .ct-noche{opacity:1}
.ct-sol{position:absolute;left:50%;top:30%;width:24vmin;height:24vmin;margin-left:-12vmin;border-radius:50%;background:#FF8A3D;border:5px solid var(--tinta);transition:transform 1.9s cubic-bezier(.5,0,.6,1),background-color 1.9s}
.cielo-transicion.anochece .ct-sol{transform:translateY(72vh);background-color:#DC0E20}
.ct-luna{position:absolute;right:17%;top:12%;width:15vmin;height:15vmin;border-radius:50%;background:#F1EAD1;border:5px solid var(--tinta);box-shadow:inset -3.2vmin -1vmin 0 #D9CFAE;transform:translateY(70vh);transition:transform 1.5s .7s cubic-bezier(.2,1.25,.4,1)}
.cielo-transicion.anochece .ct-luna{transform:none}
.ct-estrellas{position:absolute;inset:0;opacity:0;transition:opacity 1s 1.1s}
.cielo-transicion.anochece .ct-estrellas{opacity:1}
.ct-estrellas i{position:absolute;width:6px;height:6px;background:#FFF8E7;border-radius:50%;animation:titila 2s infinite}
.ct-ciudad{position:absolute;left:0;right:0;bottom:0;width:100%;height:30vh}
.ct-ciudad .ven{fill:#6E2A2E;transition:fill .4s}
.cielo-transicion.anochece .ct-ciudad .ven{fill:#FFD23F}
.globo{animation:globoSube 1.5s ease-out forwards}
@keyframes globoSube{from{transform:translateY(8px);opacity:0}15%{opacity:1}to{transform:translateY(-46px);opacity:0}}

/* ---------- Pantalla de inicio (estilo pantalla de carga) ---------- */
.escenaInicio{position:relative;height:100vh;min-height:640px;overflow:hidden;background:linear-gradient(#FFD27A,#FCB14E 55%,#F2932F)}
.rayos{position:absolute;left:50%;top:46%;width:240vmax;height:240vmax;transform:translate(-50%,-50%);animation:girar 80s linear infinite;opacity:.9}
@keyframes girar{to{transform:translate(-50%,-50%) rotate(360deg)}}
.cafeFondo{position:absolute;left:52%;bottom:13vh;height:45vh;transform:translateX(-50%);filter:drop-shadow(0 8px 0 rgba(43,10,13,.25))}
.piso{position:absolute;left:0;right:0;bottom:0;height:15vh;border-top:4px solid var(--tinta);background:repeating-conic-gradient(#F6E6C4 0 25%,#E9C98E 0 50%) 0 0/9vh 9vh}
.logoOS{position:absolute;left:2.2vw;top:2.6vh;display:flex;align-items:center;gap:10px;background:var(--rojo);border:4px solid var(--tinta);border-radius:20px;padding:8px 16px 8px 10px;box-shadow:5px 6px 0 var(--tinta);transform:rotate(-3deg);z-index:6}
.logoOS svg{width:58px;height:58px}
.logoOS b{font-family:var(--titulo);font-weight:400;color:var(--amarillo);font-size:34px;line-height:.9;letter-spacing:1px;-webkit-text-stroke:2px var(--tinta);paint-order:stroke fill;display:block}
.logoOS small{color:var(--papel);font-weight:800;font-size:13px}
.cinta{position:absolute;right:0;top:3.4vh;background:var(--rojo);color:var(--papel);font-family:var(--titulo);font-size:clamp(20px,2.4vw,34px);letter-spacing:1px;padding:8px 26px 8px 40px;border:4px solid var(--tinta);border-right:0;clip-path:polygon(0 0,100% 0,100% 100%,0 100%,22px 50%);z-index:6;-webkit-text-stroke:1.5px var(--tinta);paint-order:stroke fill;animation:cintaEntra .6s 1.1s both cubic-bezier(.2,1.4,.4,1)}
@keyframes cintaEntra{from{transform:translateX(110%)}}
.tituloJuego{position:absolute;left:50%;top:4vh;transform:translateX(-50%) rotate(-4deg);margin:0;text-align:center;z-index:5;font-family:var(--titulo);font-weight:400;line-height:.86;pointer-events:none;animation:golpe .7s .2s both cubic-bezier(.2,1.6,.4,1)}
@keyframes golpe{from{transform:translateX(-50%) rotate(-4deg) scale(2.4);opacity:0}}
.tituloJuego span{display:block;color:var(--amarillo);-webkit-text-stroke:.09em var(--tinta);paint-order:stroke fill;letter-spacing:.03em;
  text-shadow:.04em .05em 0 var(--rojo),.07em .09em 0 var(--rojoOsc),.09em .12em 0 var(--tinta)}
.tituloJuego .t1{font-size:clamp(40px,6.4vw,104px);color:var(--papel)}
.tituloJuego .t2{font-size:clamp(84px,14vw,230px);animation:flotar 3.2s 1s ease-in-out infinite}
@keyframes flotar{50%{transform:translateY(-.06em) rotate(1.5deg)}}
.chispa{position:absolute;width:28px;height:28px;z-index:6;animation:chispa 1.8s infinite}
@keyframes chispa{0%,100%{transform:scale(.2) rotate(0);opacity:0}50%{transform:scale(1) rotate(45deg);opacity:1}}
.mesero{position:absolute;left:14%;bottom:8vh;height:58vh;z-index:4;animation:correr .45s ease-in-out infinite alternate}
@keyframes correr{to{transform:translateY(-1.4vh) rotate(-2deg)}}
.mesero .pila{transform-origin:168px 66px;animation:tambalear 1.1s ease-in-out infinite alternate}
@keyframes tambalear{from{transform:rotate(-6deg)}to{transform:rotate(7deg)}}
.mesero .lineas line{animation:velocidad .5s linear infinite}
@keyframes velocidad{to{stroke-dashoffset:-40}}
.clientes{position:absolute;left:.5%;bottom:9vh;height:27vh;z-index:4;animation:susto 2.6s 1.5s ease-in-out infinite}
@keyframes susto{0%,80%,100%{transform:none}84%{transform:translateY(-1.6vh)}88%{transform:none}92%{transform:translateY(-.8vh)}}
.jonatanInicio{position:absolute;right:2vw;bottom:2vh;height:84vh;z-index:3;transform-origin:50% 100%;animation:entraJonatan .8s .5s both cubic-bezier(.2,1.3,.4,1),respirar 4s 1.4s ease-in-out infinite;
  filter:drop-shadow(0 0 0 #fff) drop-shadow(5px 0 0 #fff) drop-shadow(-5px 0 0 #fff) drop-shadow(0 -5px 0 #fff) drop-shadow(10px 12px 0 rgba(43,10,13,.35))}
@keyframes entraJonatan{from{transform:translateX(70%)}}
@keyframes respirar{50%{transform:scale(1.012,1.02)}}
.mostrador{position:absolute;right:0;bottom:0;width:44vw;height:19vh;z-index:4;border:4px solid var(--tinta);border-right:0;border-bottom:0;border-radius:22px 0 0 0;background:repeating-linear-gradient(90deg,#B8743A 0 46px,#A5652F 46px 50px);box-shadow:inset 0 14px 0 var(--rojo),inset 0 18px 0 var(--tinta)}
.mostrador:after{content:"";position:absolute;left:16px;right:0;top:34px;height:4px;background:rgba(43,10,13,.3)}
.selloCritico{position:absolute;right:30vw;top:42vh;z-index:5;border:5px solid var(--rojo);color:var(--rojo);background:rgba(255,248,231,.85);font-family:var(--titulo);font-size:clamp(22px,2.6vw,40px);padding:2px 14px;border-radius:10px;letter-spacing:2px;transform:rotate(-14deg);animation:sellar .45s 1.6s both cubic-bezier(.2,1.8,.4,1)}
@keyframes sellar{from{transform:rotate(-14deg) scale(3);opacity:0}}
.comida{position:absolute;z-index:4;animation:flotarComida 3s ease-in-out infinite alternate}
.comida svg{width:100%;height:100%;filter:drop-shadow(3px 4px 0 rgba(43,10,13,.3))}
@keyframes flotarComida{from{transform:translateY(0) rotate(-12deg)}to{transform:translateY(-3vh) rotate(14deg)}}
.pizarra{position:absolute;left:2vw;top:17vh;width:255px;z-index:5;background:#24352B;border:10px solid #8B5A2B;outline:3px solid var(--tinta);border-radius:12px;color:#F4F1E4;padding:10px 14px 12px;box-shadow:6px 8px 0 rgba(43,10,13,.4);transform:rotate(2deg);font-family:'Segoe Print','Comic Sans MS','Chalkboard SE',var(--texto)}
.pizarra h3{margin:0 0 6px;font-size:20px;text-align:center;border-bottom:2px dashed rgba(244,241,228,.5);padding-bottom:4px}
.pizarra ol{margin:0;padding-left:22px;font-size:14.5px}
.pizarra li{margin:3px 0}
.pizarra li span{float:right;color:#FFD23F}
.pizarra .vacio{color:#CFE0D3}
.panelInicio{position:absolute;left:50%;bottom:3.2vh;transform:translateX(-50%);width:min(560px,92vw);z-index:8;text-align:center}
.historia{background:var(--papel);border:4px solid var(--tinta);border-radius:18px;padding:10px 16px;font-weight:800;font-size:clamp(15px,1.4vw,19px);box-shadow:5px 6px 0 var(--tinta);min-height:58px;display:flex;align-items:center;justify-content:center}
.historia.cambia{animation:pop .35s}
@keyframes pop{0%{transform:scale(.92)}60%{transform:scale(1.04)}}
.carga{margin-top:12px;height:34px;border:4px solid var(--tinta);border-radius:18px;background:#3B0F12;position:relative;overflow:hidden;box-shadow:4px 5px 0 var(--tinta)}
.carga i{position:absolute;left:0;top:0;bottom:0;width:0;background:repeating-linear-gradient(-45deg,var(--amarillo) 0 16px,#FFE36E 16px 32px);transition:width .15s linear}
.carga span{position:relative;font-family:var(--titulo);color:#fff;font-size:22px;line-height:26px;-webkit-text-stroke:1.5px var(--tinta);paint-order:stroke fill}
.saltar{display:block;margin-top:6px;font-weight:800;font-size:12.5px;color:var(--tinta)}
.btnJugar{font-family:var(--titulo);font-weight:400;font-size:clamp(46px,5vw,72px);letter-spacing:3px;padding:6px 64px;margin-top:12px;background:var(--amarillo);border:5px solid var(--tinta);border-radius:22px;box-shadow:0 9px 0 var(--tinta);color:#fff;-webkit-text-stroke:3px var(--tinta);paint-order:stroke fill;text-shadow:3px 4px 0 var(--tinta);animation:latirJugar 1s ease-in-out infinite;overflow:hidden;position:relative}
.btnJugar:after{content:"";position:absolute;top:-20%;bottom:-20%;width:40px;left:-60px;background:rgba(255,255,255,.55);transform:skewX(-20deg);animation:brillo 2.2s infinite}
@keyframes brillo{0%,60%{left:-60px}100%{left:110%}}
@keyframes latirJugar{50%{transform:scale(1.06)}}
.btnJugar:active{transform:translateY(5px) scale(1);box-shadow:0 3px 0 var(--tinta)}
@media(max-width:900px){.pizarra{display:none}.clientes{display:none}.mesero{left:4%;height:44vh}.jonatanInicio{height:60vh}.mostrador{width:60vw;height:14vh}.selloCritico{right:42vw;top:48vh}}
@media(prefers-reduced-motion:reduce){.rayos{animation:none}}
</style>
</head>
<body class="en-inicio">
<svg width="0" height="0" style="position:absolute" aria-hidden="true"><defs id="defs"></defs></svg>
<header>
  <div class="hd">
    <div class="logo"><svg viewBox="0 0 64 64"><use href="#logoOS"/></svg><div><b>ORDERSTACK</b><small>Restaurant Rush</small></div></div>
    <div class="kpis" id="kpis"></div>
    <button class="hbtn" id="bAyuda" title="Cómo se juega (H)">? Ayuda</button>
    <button class="hbtn" id="bDatos" title="Datos del juego">Datos</button>
  </div>
</header>
<div class="cuadros"></div>
<main>
  <!-- ================= INICIO ================= -->
  <section class="pantalla" id="pInicio">
    <div class="escenaInicio" id="escenaInicio">
      <svg class="rayos" id="rayos" viewBox="-100 -100 200 200" aria-hidden="true"></svg>
      <svg class="cafeFondo" id="cafeFondo" viewBox="0 0 600 420" aria-hidden="true"></svg>
      <div class="piso"></div>
      <div class="logoOS"><svg viewBox="0 0 64 64"><use href="#logoOS"/></svg><div><b>ORDERSTACK</b><small>la cafetería de moda</small></div></div>
      <div class="cinta">¡LLEGÓ EL CRÍTICO!</div>
      <h1 class="tituloJuego"><span class="t1">RESTAURANT</span><span class="t2">RUSH</span></h1>
      <div class="pizarra"><h3>Los mejores</h3><div id="rankingInicio"></div></div>
      <svg class="clientes" id="clientesInicio" viewBox="0 0 300 240" aria-hidden="true"></svg>
      <svg class="mesero" id="meseroInicio" viewBox="0 0 230 360" aria-hidden="true"></svg>
      <img class="jonatanInicio" src="{{JONATAN_CUERPO}}" alt="El Gran Crítico Jonatan">
      <div class="selloCritico">CRÍTICO</div>
      <div class="mostrador"></div>
      <div id="comidaInicio"></div>
      <div class="panelInicio">
        <div class="historia" id="historia">Cargando la cafetería...</div>
        <div class="carga" id="carga"><i id="cargaBarra"></i><span id="cargaPct">0%</span></div>
        <button class="btnJugar oculto" id="bJugar">JUGAR</button>
        <span class="saltar" id="saltar">Haz clic o pulsa Espacio para saltar la historia</span>
      </div>
    </div>
  </section>

  <!-- ================= DÍA ================= -->
  <section class="pantalla" id="pDia">
    <div class="dia">
      <div>
        <div class="card"><h2>Fila de espera <small id="filaCuenta"></small></h2><p class="sub">Haz clic en una mesa libre (o pulsa su número) para sentar al grupo marcado.</p><div id="fila"></div></div>
        <div class="card"><h2>Cuentas por cobrar</h2><div id="cuentas"></div></div>
      </div>
      <div class="centro">
        <div class="lienzo"><canvas id="mapa" width="1440" height="960" aria-label="Mapa del restaurante"></canvas><div class="cartel" id="cartel"><b id="cartelT"></b><span id="cartelS"></span></div></div>
        <div class="reloj"><svg id="relojSvg" aria-hidden="true"></svg><div class="lado"><div class="tiempo" id="tiempo"></div><button class="btn cr chico oculto" id="bCerrar">Cerrar temprano</button></div></div>
        <div class="perdidas oculto" id="perdidas"></div>
        <div class="estado"><div><span id="eManoIco"></span><span><b>En tus manos</b><span id="eMano"></span></span></div><div><span id="eCocinaIco"></span><span><b>Cocina</b><span id="eCocina"></span></span></div><div><span><b>Hoy</b><span id="eHoy"></span></span></div></div>
        <div class="aviso" id="aviso">¡Bienvenido! Sienta al primer grupo y ve a la cocina.</div>
        <div class="teclas"><span><kbd>←</kbd><kbd>↑</kbd><kbd>→</kbd><kbd>↓</kbd> caminar</span><span><kbd>E</kbd> usar cocina, mesa o caja</span><span><kbd>Espacio</kbd> esperar</span><span><kbd>1</kbd>…<kbd>6</kbd> sentar en esa mesa</span><span><kbd>H</kbd> ayuda</span></div>
      </div>
      <div>
        <div class="card"><h2>Jonatan</h2><div class="jonatan"><div class="retrato"><img src="{{JONATAN_BUSTO}}" alt="Jonatan"></div><div class="burbuja" id="frase"></div></div>
          <div class="amenaza"><div class="lbl"><span>Amenaza</span><span id="amenazaTxt"></span></div><div class="linea"><i id="amenazaBarra"></i></div></div></div>
        <div class="card"><h2>Pedidos <small>el más urgente arriba</small></h2><div id="pedidos"></div></div>
        <div class="card"><h2>Lo que pasó</h2><div class="eventos" id="eventos"></div></div>
      </div>
    </div>
  </section>

  <!-- ================= NOCHE ================= -->
  <section class="pantalla" id="pNoche">
    <div class="noche">
      <div>
        <div class="ciudad"><svg id="ciudad" viewBox="0 0 760 500" aria-label="Mapa de la ciudad"></svg><div class="cartel" id="cartelNoche"><b id="cartelNocheT"></b><span id="cartelNocheS"></span></div></div>
        <div class="reloj"><svg id="relojNocheSvg" aria-hidden="true"></svg><div class="lado"><div class="tiempo" id="tiempoNoche"></div></div></div>
        <div class="aviso" id="avisoNoche">Elige un pedido y traza tu ruta haciendo clic en las esquinas iluminadas.</div>
      </div>
      <div>
        <div class="desvio oculto" id="desvio"></div>
        <div class="card"><h2>Domicilios <small id="motoTxt"></small></h2><div id="domicilios"></div></div>
        <div class="card"><h2>Tu ruta <small id="rutaTxt"></small></h2><div id="ruta"></div>
          <div class="botones">
            <button class="btn am" id="bSalir">Salir con la moto</button>
            <button class="btn cr chico" id="bDeshacer" title="Borrar el último lugar (tecla Retroceso)">Deshacer</button>
            <button class="btn cr chico" id="bBorrar">Borrar ruta</button>
            <button class="btn cr chico" id="bRegreso" title="Completa el camino más corto de vuelta al restaurante">Completar regreso</button>
            <button class="btn cr chico" id="bGps" title="Muestra la ruta más rápida, pero ese viaje ya no da bono">Ver GPS</button>
          </div></div>
        <div class="card"><h2>Fin de la noche</h2><div id="finNoche"></div></div>
        <div class="card"><h2>Jonatan</h2><div class="jonatan"><div class="retrato"><img src="{{JONATAN_BUSTO}}" alt="Jonatan"></div><div class="burbuja" id="fraseNoche"></div></div></div>
      </div>
    </div>
  </section>

  <!-- ================= TIENDA ================= -->
  <section class="pantalla" id="pTienda">
    <div class="tienda">
      <div class="card"><h2>Mejoras <small>máximo una por jornada</small></h2><p class="sub" id="tiendaSub"></p><div class="mejoras" id="mejoras"></div>
        <div class="acciones"><button class="btn am grande" id="bSiguiente">Siguiente jornada</button></div></div>
      <div>
        <div class="card"><h2>Resumen de la jornada</h2><div id="resumen"></div></div>
        <div class="card"><h2>Jonatan</h2><div class="jonatan"><div class="retrato"><img src="{{JONATAN_BUSTO}}" alt="Jonatan"></div><div class="burbuja" id="fraseTienda"></div></div></div>
      </div>
    </div>
  </section>

  <!-- ================= FINAL: LA RESEÑA ================= -->
  <section class="pantalla" id="pFinal">
    <div class="final">
      <div>
        <div class="periodico">
          <div class="cab"><b>La Gaceta del Sabor</b><small>Sección de restaurantes · Edición especial</small></div>
          <h3 id="titular"></h3>
          <div class="estrellas" id="estrellas"></div>
          <p id="resena"></p>
          <p class="firma">El Gran Crítico Jonatan</p>
          <div class="sello" id="sello"></div>
        </div>
        <div class="card" style="margin-top:24px"><p class="resultado" id="resultado"></p><div class="cifras" id="cifras"></div><table class="t" id="historial"></table>
          <div class="acciones"><button class="btn am grande" id="bOtra">Jugar otra vez</button></div></div>
      </div>
      <div>
        <div class="jefe"><div class="burbuja" id="fraseFinal"></div><img src="{{JONATAN_CUERPO}}" alt="El Gran Crítico Jonatan"></div>
        <div class="card" style="margin-top:16px"><h2>Ranking</h2><div id="rankingFinal"></div></div>
      </div>
    </div>
  </section>
</main>
<div class="cuadros"></div>

<!-- ================= ESCENAS (transiciones, ataques, avisos) ================= -->
<div class="cielo-transicion" id="cieloT" aria-hidden="true">
  <div class="ct-capa ct-tarde"></div><div class="ct-capa ct-noche"></div><div class="ct-estrellas" id="ctEstrellas"></div>
  <div class="ct-sol"></div><div class="ct-luna"></div>
  <svg class="ct-ciudad" viewBox="0 0 1200 220" preserveAspectRatio="none" id="ctCiudad"></svg>
</div>
<div class="escena" id="escena" role="dialog" aria-modal="true"><div class="card" id="escenaCard"></div></div>
<div class="cutin" id="cutin" aria-live="assertive"><div class="franja"></div><img src="{{JONATAN_CUERPO}}" alt=""><div class="txt"><b id="cutinT"></b><span id="cutinS"></span></div></div>
<div class="brindis" id="brindis" aria-live="polite"></div>

<!-- ================= MODALES ================= -->
<div class="modal" id="mNombre"><div class="card">
  <h2>¿Cómo te llamas, mesero?</h2>
  <p class="sub">Tu nombre aparece en el ranking si entras a los 5 mejores.</p>
  <div class="campo"><input id="nombre" maxlength="20" placeholder="Tu nombre" autocomplete="off"><button class="btn am" id="bEmpezar">¡A trabajar!</button></div>
  <p class="sub" style="margin-top:12px">De día: <kbd>flechas</kbd> para caminar, <kbd>E</kbd> para usar la cocina, las mesas y la caja, <kbd>Espacio</kbd> para esperar. De noche: traza tus rutas en el mapa. <kbd>H</kbd> abre la ayuda.</p>
</div></div>
<div class="modal" id="mAyuda"><div class="card">
  <h2>Cómo se juega <small><button class="btn cr chico" data-cerrar>Cerrar</button></small></h2>
  <p><b>Meta:</b> terminar la jornada 5 con reputación de 40 o más y $1.200.000 en caja. Si la reputación llega a 0, pierdes.</p>
  <p><b>El tiempo:</b> solo avanza cuando actúas. Cada paso, cada uso de <kbd>E</kbd>, cada espera y cada grupo que sientas cuesta 1 tick. El día dura 150 ticks y en los últimos 25 ya no entran clientes.</p>
  <ul>
    <li><b>Sentar:</b> haz clic en una mesa libre (o pulsa su número). Primero pasan los VIP, pero cada 2 VIP entra un cliente común.</li>
    <li><b>Cocinar:</b> con <kbd>E</kbd> junto a la cocina se empieza el pedido más urgente. Cuando esté listo, otra <kbd>E</kbd> lo recoge.</li>
    <li><b>Servir y cobrar:</b> lleva el plato a su mesa y pulsa <kbd>E</kbd>. Comen 3 ticks y dejan la cuenta. <kbd>E</kbd> en la caja cobra todas.</li>
    <li><b>Al cerrar:</b> las cuentas sin cobrar se pierden, los que comían se van sin pagar y cada mesa sin servir resta 6 de reputación.</li>
    <li><b>Noche:</b> elige 1 pedido (2 con la moto mejorada) y traza tu ruta haciendo clic en las esquinas. El GPS te dice cuánto tarda la ruta más rápida. Si la igualas sin mirarlo, ganas $5.000 de bono.</li>
    <li><b>Calles cerradas:</b> desde la jornada 2, Jonatan puede cerrar la calle que tienes adelante. La moto se detiene y trazas un desvío.</li>
    <li><b>Pedido urgente:</b> una vez por noche puede llegar uno que paga el doble pero con poco tiempo. Puedes rechazarlo sin castigo.</li>
    <li><b>Jonatan:</b> cada error llena su barra de amenaza. Al llegar a 100 ataca: Avalancha, Tráfico, Caos o Auditoría.</li>
    <li><b>Mejoras:</b> al terminar cada jornada puedes comprar una. Sin mejoras es muy difícil ganar.</li>
  </ul>
</div></div>
<div class="modal" id="mDatos"><div class="card">
  <h2>Datos del juego <small><button class="btn cr chico" data-cerrar>Cerrar</button></small></h2>
  <div id="datos"></div>
</div></div>

<script>
// =====================================================================
// BASE: comunicación con el servidor (C++), avisos y despacho de pantallas
// La página solo DIBUJA. Toda la lógica del juego está en Juego.cpp.
// =====================================================================
const $ = s => document.querySelector(s);
let E = null;                          // Último estado recibido de C++
let enviando = false, pendientes = []; // Acciones en fila (si se presionan teclas rápido)
let ocupado = false;                   // true mientras hay una escena o animación: las teclas del juego no hacen nada
let pausaTeclas = 0;                   // Después de un aviso de cierre, las teclas del juego esperan un segundo
const TINTA = '#2B0A0D';
const TIPOS = ['Común', 'Frecuente', 'Amigo del gerente', 'Prestigio'];
const COLOR_TIPO = ['#8A6A55', '#2F7DD1', '#8E44AD', '#C99700'];
const pesos = n => (n < 0 ? '-' : '') + '$' + Math.abs(Math.round(n)).toLocaleString('es-CO');
const esc = s => String(s).replace(/[&<>"]/g, c => ({'&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;'}[c]));
const espera = ms => new Promise(r => setTimeout(r, ms));
const limitar = (x, a, b) => Math.max(a, Math.min(b, x));
const plural = (n, uno, varios) => n + ' ' + (n === 1 ? uno : varios);

async function api(ruta, datos) {      // Llama a C++ y guarda el estado nuevo (sin dibujar)
  const r = await fetch(ruta, {method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: new URLSearchParams(datos || {})});
  const j = await r.json();
  E = j.estado;
  return j;
}
async function accion(ruta, datos) {   // Acción normal: llama, avisa y dibuja. Van una por una
  if (enviando) { if (pendientes.length < 3) pendientes.push([ruta, datos]); return null; }
  enviando = true;
  let j = null;
  try { j = await api(ruta, datos); if (j.mensaje) avisar(j.mensaje, j.ok); dibujar(); }
  catch (e) { console.error(e); avisar('No hay conexión con el programa. ¿Sigue abierta la ventana del juego?', false); }
  enviando = false;
  if (pendientes.length && !ocupado) { const [r, d] = pendientes.shift(); accion(r, d); }
  return j;
}
function avisar(msg, ok) {
  const el = E && E.fase === 'noche' ? $('#avisoNoche') : $('#aviso');
  el.textContent = msg;
  el.classList.toggle('mal', !ok);
}
let temporizadorBrindis = null;
function brindis(texto, oro) {         // Mensaje que sube desde abajo unos segundos
  const b = $('#brindis');
  b.innerHTML = texto;
  b.classList.toggle('oro', !!oro);
  b.classList.add('on');
  clearTimeout(temporizadorBrindis);
  temporizadorBrindis = setTimeout(() => b.classList.remove('on'), 3200);
}

// ---------- Encabezado ----------
function kpis() {
  if (!E || E.fase === 'inicio') { $('#kpis').innerHTML = ''; return; }
  const rep = E.reputacion, colorRep = rep >= 60 ? 'var(--verde)' : rep >= E.metaRep ? 'var(--naranjaOsc)' : 'var(--rojo)';
  const dinPct = limitar(100 * E.dinero / E.metaDinero, 0, 100);
  $('#kpis').innerHTML =
    `<div class="kpi">Jornada ${E.jornada} de 5<b>${esc(E.nombreJornada)}</b></div>` +
    `<div class="kpi">Dinero (meta ${pesos(E.metaDinero)})<b>${pesos(E.dinero)}</b><div class="barra"><i style="width:${dinPct}%;background:var(--amarillo)"></i></div></div>` +
    `<div class="kpi">Reputación (meta ${E.metaRep})<b>${rep}</b><div class="barra"><i style="width:${rep}%;background:${colorRep}"></i></div></div>` +
    `<div class="kpi">Puntaje<b>${E.puntaje.toLocaleString('es-CO')}</b></div>`;
}

// ---------- Despacho: qué pantalla se dibuja y qué transición toca ----------
let faseAnterior = null, tickAnterior = null, ataquesAnterior = 0, fraseAnterior = '';
function mostrar(id) { document.querySelectorAll('.pantalla').forEach(p => p.classList.toggle('on', p.id === id)); }
function frases() {
  const f = '«' + E.jonatan.frase + '»';
  ['#frase', '#fraseNoche', '#fraseTienda'].forEach(s => { $(s).textContent = f; });
  if (E.jonatan.frase !== fraseAnterior) {
    ['#frase', '#fraseNoche', '#fraseTienda'].forEach(s => { const b = $(s); b.classList.remove('nueva'); void b.offsetWidth; b.classList.add('nueva'); });
    fraseAnterior = E.jonatan.frase;
  }
}
function dibujar() {
  if (!E) return;
  const fase = E.fase, antes = faseAnterior;
  document.body.classList.toggle('en-inicio', fase === 'inicio');
  document.body.classList.toggle('de-noche', fase === 'noche');
  kpis();
  frases();
  if (fase === 'inicio') { mostrar('pInicio'); dibujarInicio(antes); }
  else if (fase === 'dia') { mostrar('pDia'); dibujarDia(); }
  else if (fase === 'noche') { mostrar('pNoche'); if (antes !== 'noche') reiniciarNoche(); dibujarNoche(); }
  else if (fase === 'tienda') { mostrar('pTienda'); dibujarTienda(); }
  else { mostrar('pFinal'); dibujarFinal(); }
  if (antes !== fase) window.scrollTo(0, 0);
  if (fase === 'dia' && antes === 'dia') {
    revisarAvisos(tickAnterior, E.tick);                       // Avisos de 30, 25, 15 y 5 ticks
    if (E.numAtaques > ataquesAnterior) anunciarAtaque();      // Jonatan entra en grande
  }
  tickAnterior = E.tick;
  ataquesAnterior = E.numAtaques;
  faseAnterior = fase;
  if (antes !== null && antes !== fase) transicion(antes, fase);
  else if (antes === null && fase === 'final') animarResena();
}

// ---------- Escenas: tarjeta grande en el centro que espera un botón ----------
let resolverEscena = null;
function abrirEscena(html, botones) {  // botones: [{texto, clase, valor}] → devuelve una promesa con el valor del botón
  ocupado = true;
  pendientes = [];
  const card = $('#escenaCard');
  card.innerHTML = html + `<div class="acciones">${botones.map((b, i) => `<button class="btn ${b.clase || 'am'}" data-i="${i}">${b.texto}</button>`).join('')}</div>`;
  $('#escena').classList.add('on');
  return new Promise(res => {
    resolverEscena = i => {
      $('#escena').classList.remove('on');
      ocupado = false;
      resolverEscena = null;
      res(botones[i].valor);
    };
    card.querySelectorAll('[data-i]').forEach(b => b.addEventListener('click', () => resolverEscena && resolverEscena(+b.dataset.i)));
    setTimeout(() => { const ult = card.querySelector('.acciones .btn:last-child'); if (ult) ult.focus(); }, 60);
  });
}
function cartel(id, titulo, sub, peligro, ms) {    // Aviso grande sobre el mapa (día o noche)
  const c = $('#' + id);
  $('#' + id + 'T').textContent = titulo;
  $('#' + id + 'S').textContent = sub;
  c.classList.toggle('peligro', !!peligro);
  c.classList.remove('on'); void c.offsetWidth; c.classList.add('on');
  clearTimeout(c._t);
  c._t = setTimeout(() => c.classList.remove('on'), ms || 2300);
}
async function cutin(titulo, sub, ms) {            // Jonatan entra en grande con un texto (ataques y calles cerradas)
  $('#cutinT').textContent = titulo;
  $('#cutinS').textContent = sub;
  const c = $('#cutin');
  c.classList.add('on');
  await espera(ms || 1900);
  c.classList.remove('on');
  await espera(250);
}

async function transicion(antes, fase) {
  if (fase === 'dia' && (antes === 'inicio' || antes === 'tienda' || antes === 'final')) await escenaJornada();
  else if (antes === 'dia' && fase === 'noche') await escenaCierreDia();
  else if (antes === 'dia' && fase === 'final') { await escenaCierreDia(true); animarResena(); }
  else if (antes === 'noche' && (fase === 'tienda' || fase === 'final')) { await escenaFinNoche(fase === 'final'); if (fase === 'final') animarResena(); }
}

// =====================================================================
// DIBUJOS EN SVG (hechos a mano, estilo caricatura con contorno grueso)
// La comida se usa en HTML (<use>) y en el mapa del canvas (como imagen).
// =====================================================================
const C = 'stroke="#2B0A0D" stroke-width="3" stroke-linejoin="round" stroke-linecap="round"';
const COMIDA = {
  'Café': `<ellipse cx="30" cy="53" rx="24" ry="6" fill="#F1EAD1"/><path d="M48 29a8 8 0 1 1 0 14" fill="none"/><path d="M12 24h36l-4 23a6 6 0 0 1-6 5H22a6 6 0 0 1-6-5z" fill="#FFF8E7"/><ellipse cx="30" cy="24" rx="18" ry="4.5" fill="#6B3A1E"/><path d="M24 16c-3-4 3-6 0-10M35 16c-3-4 3-6 0-10" fill="none" stroke-width="2.5"/>`,
  'Limonada': `<path d="M15 14h34l-5 40a5 5 0 0 1-5 4H25a5 5 0 0 1-5-4z" fill="#E9F7FF"/><path d="M17.5 26h29l-3.6 28a5 5 0 0 1-5 4H26a5 5 0 0 1-5-4z" fill="#FFE45C"/><path d="M41 3l-7 30" fill="none" stroke-width="7"/><path d="M41 3l-7 30" fill="none" stroke="#DC0E20" stroke-width="3"/><circle cx="15" cy="15" r="9" fill="#FFE45C"/><path d="M15 6v18M6 15h18" fill="none" stroke-width="1.5"/>`,
  'Jugo natural': `<path d="M15 14h34l-5 40a5 5 0 0 1-5 4H25a5 5 0 0 1-5-4z" fill="#E9F7FF"/><path d="M17.5 24h29l-3.8 30a5 5 0 0 1-5 4H26a5 5 0 0 1-5-4z" fill="#FF9F1C"/><path d="M27 3l5 30" fill="none" stroke-width="7"/><path d="M27 3l5 30" fill="none" stroke="#4E9A2F" stroke-width="3"/><circle cx="49" cy="15" r="9" fill="#FF9F1C"/><path d="M49 6v18M40 15h18" fill="none" stroke-width="1.5"/>`,
  'Papas fritas': `<g fill="#FFD23F"><rect x="17" y="8" width="7" height="30" rx="2"/><rect x="26" y="3" width="7" height="34" rx="2"/><rect x="35" y="6" width="7" height="32" rx="2"/><rect x="43" y="11" width="7" height="27" rx="2"/></g><path d="M11 26h42l-6 31H17z" fill="#DC0E20"/><path d="M23 37c3 6 15 6 18 0" fill="none" stroke="#FFD23F" stroke-width="3.5"/>`,
  'Sándwich': `<path d="M7 42L32 9l25 33z" fill="#F4D7A1"/><path d="M5 42c5 6 9-2 14 2s9-2 13 2 9-2 13 2 9-2 14 0v5H5z" fill="#7BC453"/><rect x="6" y="49" width="52" height="6" rx="2" fill="#E2483C"/><path d="M5 55h54v4a3 3 0 0 1-3 3H8a3 3 0 0 1-3-3z" fill="#F4D7A1"/>`,
  'Hamburguesa': `<path d="M7 30a25 19 0 0 1 50 0z" fill="#E8A13A"/><g fill="#FFF8E7" stroke="none"><ellipse cx="22" cy="20" rx="2.4" ry="1.4"/><ellipse cx="32" cy="16" rx="2.4" ry="1.4"/><ellipse cx="42" cy="21" rx="2.4" ry="1.4"/><ellipse cx="30" cy="24" rx="2.4" ry="1.4"/></g><path d="M5 33c5 6 8-1 13 3s8-2 13 2 8-2 13 2 8-2 15-2v3H5z" fill="#7BC453"/><path d="M8 37h48l-4 7-5-4-4 5-6-5-5 5-5-5-5 5-5-4-4 4z" fill="#FFD23F"/><rect x="6" y="40" width="52" height="10" rx="5" fill="#6B3A1E"/><path d="M7 51h50v2a7 7 0 0 1-7 7H14a7 7 0 0 1-7-7z" fill="#E8A13A"/>`,
  'Ensalada': `<circle cx="21" cy="28" r="10" fill="#7BC453"/><circle cx="33" cy="23" r="11" fill="#5FAE3C"/><circle cx="44" cy="29" r="9" fill="#7BC453"/><circle cx="29" cy="32" r="5" fill="#E2483C"/><circle cx="40" cy="34" r="4" fill="#FFD23F"/><path d="M5 34h54a27 22 0 0 1-54 0z" fill="#FFF8E7"/><path d="M13 42h38" fill="none" stroke="#2F7DD1" stroke-width="3"/>`,
  'Postre': `<path d="M8 30L44 14l12 16z" fill="#FFF8E7"/><path d="M8 30h48v22H8z" fill="#F7B6C8"/><path d="M8 39h48" fill="none" stroke="#FFF8E7" stroke-width="5"/><path d="M8 36.5h48M8 41.5h48" fill="none" stroke-width="1.5"/><path d="M44 10c2-5 6-7 9-7" fill="none" stroke-width="2.5"/><circle cx="44" cy="12" r="6" fill="#DC0E20"/>`,
  'Pasta': `<ellipse cx="32" cy="43" rx="28" ry="14" fill="#FFF8E7"/><ellipse cx="32" cy="37" rx="21" ry="12" fill="#FFD966"/><path d="M17 37c6-7 24-7 30 0M19 42c7 4 19 4 26 0M23 32c4-3 14-3 18 0" fill="none" stroke-width="2"/><path d="M23 31c3-7 15-7 17 0s-4 8-9 7-10-2-8-7z" fill="#E2483C"/><path d="M35 25c4-5 9-3 9 2-5 2-7 1-9-2z" fill="#4E9A2F"/>`,
  'Pizza': `<path d="M8 15Q32 4 56 15L32 60z" fill="#FFD23F"/><path d="M8 15Q32 4 56 15l-2.5 5.5Q32 11 10.5 20.5z" fill="#E8A13A"/><circle cx="27" cy="27" r="5" fill="#DC0E20"/><circle cx="38" cy="33" r="4.5" fill="#DC0E20"/><circle cx="30" cy="43" r="4" fill="#DC0E20"/>`
};
const ICONO_MEJORA = [   // Mesero, Mejor cocina, Moto mejorada, Mejor ambiente
  `<circle cx="32" cy="38" r="15" fill="#F6C79E"/><path d="M15 31a17 14 0 0 1 34 0z" fill="#2F7DD1"/><path d="M44 31h12" fill="none" stroke-width="4"/><circle cx="26" cy="38" r="2" fill="#2B0A0D" stroke="none"/><circle cx="38" cy="38" r="2" fill="#2B0A0D" stroke="none"/><path d="M26 45q6 5 12 0" fill="none"/><ellipse cx="32" cy="10" rx="20" ry="4" fill="#C9CED8"/>`,
  `<path d="M32 4c6 12 18 16 18 32a18 18 0 0 1-36 0c0-9 5-13 7-19 4 5 5 8 8 9-1-9 0-15 3-22z" fill="#FF7A1A"/><path d="M32 30c4 6 9 8 9 15a9 9 0 0 1-18 0c0-5 4-8 9-15z" fill="#FFD23F"/>`,
  `<circle cx="15" cy="46" r="9" fill="#3B3B46"/><circle cx="49" cy="46" r="9" fill="#3B3B46"/><path d="M8 44c2-10 10-14 22-14h8l6-12h6l-6 14c6 0 10 5 10 12H24c0-6-6-8-16 0z" fill="#DC0E20"/><rect x="6" y="18" width="18" height="14" rx="3" fill="#FFD23F"/><circle cx="15" cy="46" r="3" fill="#C9CED8"/><circle cx="49" cy="46" r="3" fill="#C9CED8"/>`,
  `<path d="M22 46V14l26-6v32" fill="none" stroke-width="4"/><ellipse cx="16" cy="46" rx="8" ry="6" fill="#8E44AD"/><ellipse cx="42" cy="40" rx="8" ry="6" fill="#8E44AD"/><path d="M22 20l26-6" fill="none" stroke-width="6"/>`
];
const ESTRELLA = `<path d="M32 4l8.5 17.5 19.3 2.8-14 13.6 3.3 19.2L32 48l-17.2 9.1 3.3-19.2-14-13.6 19.3-2.8z"/>`;
const LOGO = `<circle cx="32" cy="32" r="29" fill="#FFF8E7"/><ellipse cx="32" cy="50" rx="20" ry="4.5" fill="#C9CED8"/><path d="M14 33a18 13 0 0 1 36 0z" fill="#E8A13A"/><path d="M12 35c4 5 7-1 10 2s7-1 10 2 7-1 10 2 6-2 10-2v2H12z" fill="#7BC453"/><rect x="13" y="38" width="38" height="7" rx="3.5" fill="#6B3A1E"/><path d="M14 45h36v1a4 4 0 0 1-4 4H18a4 4 0 0 1-4-4z" fill="#E8A13A"/><path d="M22 14c-2-3 2-5 0-8M32 14c-2-3 2-5 0-8M42 14c-2-3 2-5 0-8" fill="none" stroke-width="2.5"/>`;
const MOTO = `<g ${C}><circle cx="-16" cy="10" r="8" fill="#3B3B46"/><circle cx="18" cy="10" r="8" fill="#3B3B46"/><path d="M-24 8c2-9 9-12 19-12h9l6-10h6l-6 13c6 0 10 4 10 9H-8c0-5-6-6-16 0z" fill="#DC0E20"/><rect x="-28" y="-20" width="17" height="14" rx="3" fill="#FFD23F"/><path d="M-21 -13h4" fill="none" stroke-width="2"/><circle cx="2" cy="-18" r="7" fill="#FFF8E7"/><path d="M-5 -19a7 7 0 0 1 14 0z" fill="#DC0E20"/><circle cx="-16" cy="10" r="2.5" fill="#C9CED8"/><circle cx="18" cy="10" r="2.5" fill="#C9CED8"/></g>`;

const svgSimbolo = (id, interior) => `<symbol id="${id}" viewBox="0 0 64 64"><g ${C}>${interior}</g></symbol>`;
const idComida = nombre => 'f' + Object.keys(COMIDA).indexOf(nombre);
const usarComida = (nombre, clase) => COMIDA[nombre] ? `<svg class="${clase || 'ico'}" viewBox="0 0 64 64" aria-hidden="true"><use href="#${idComida(nombre)}"/></svg>` : '';
function prepararSimbolos() {          // Mete todos los dibujos en <defs> para usarlos con <use>
  let s = svgSimbolo('logoOS', LOGO);
  Object.keys(COMIDA).forEach(n => { s += svgSimbolo(idComida(n), COMIDA[n]); });
  ICONO_MEJORA.forEach((m, i) => { s += svgSimbolo('m' + i, m); });
  s += `<symbol id="estrella" viewBox="0 0 64 64">${ESTRELLA}</symbol>`;
  $('#defs').innerHTML = s;
}
const IMG_COMIDA = {};                 // La misma comida como imagen para el canvas
function prepararImagenes() {
  Object.keys(COMIDA).forEach(n => {
    const img = new Image();
    img.src = 'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(`<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="128" height="128"><g ${C}>${COMIDA[n]}</g></svg>`);
    IMG_COMIDA[n] = img;
  });
}

// ---------- Caritas para la fila (24×24) ----------
const PIELES = ['#F6C79E', '#E8B48A', '#C98E62', '#9A6440', '#FADCC0'];
const PELOS = ['#3B2A20', '#6B4423', '#1E1A1A', '#C98A3B', '#8A2D1E'];
function cara(semilla, animo) {        // animo: 2 feliz, 1 normal, 0 enojado
  const piel = PIELES[semilla % 5], pelo = PELOS[(semilla * 3) % 5];
  const boca = animo === 2 ? '<path d="M8 15q4 4 8 0" fill="none"/>' : animo === 1 ? '<path d="M8.5 15.5h7" fill="none"/>' : '<path d="M8 17q4-4 8 0" fill="none"/>';
  const cejas = animo === 0 ? '<path d="M6.5 8l3.5 1.5M17.5 8L14 9.5" fill="none"/>' : '';
  return `<svg viewBox="0 0 24 24" aria-hidden="true"><g stroke="#2B0A0D" stroke-width="1.6" stroke-linecap="round"><circle cx="12" cy="13" r="9.5" fill="${animo === 0 ? '#F08A7A' : piel}"/><path d="M3 11a9.5 9 0 0 1 18 0c-3-3-6-3.5-9-3s-6 0-9 3z" fill="${pelo}"/><circle cx="8.5" cy="12" r="1.2" fill="#2B0A0D" stroke="none"/><circle cx="15.5" cy="12" r="1.2" fill="#2B0A0D" stroke="none"/>${cejas}${boca}</g></svg>`;
}
function estrellaSVG(llena) {
  return `<svg viewBox="0 0 64 64" aria-hidden="true"><g stroke="#2B0A0D" stroke-width="3.5" stroke-linejoin="round" fill="${llena ? '#FFD23F' : '#D8CBB0'}">${ESTRELLA}</g></svg>`;
}

// ---------- Personajes grandes de la pantalla de inicio ----------
function extremidad(puntos, ancho, color) {    // Brazo o pierna: trazo de tinta y encima el color (así queda el contorno)
  const d = 'M' + puntos.map(p => p.join(' ')).join(' L');
  return `<path d="${d}" fill="none" stroke="#2B0A0D" stroke-width="${ancho + 6}" stroke-linecap="round" stroke-linejoin="round"/><path d="${d}" fill="none" stroke="${color}" stroke-width="${ancho}" stroke-linecap="round" stroke-linejoin="round"/>`;
}
function dibujarMeseroInicio() {
  const platos = [0, 1, 2, 3, 4].map(i => `<ellipse cx="${166 + (i % 2 ? 3 : -3)}" cy="${52 - i * 9}" rx="34" ry="7" fill="#FFF8E7"/>`).join('');
  $('#meseroInicio').innerHTML = `
    <g class="lineas" stroke="#2B0A0D" stroke-width="5" stroke-linecap="round" stroke-dasharray="22 18"><line x1="4" y1="170" x2="54" y2="170"/><line x1="12" y1="200" x2="58" y2="200"/><line x1="2" y1="232" x2="48" y2="232"/></g>
    <ellipse cx="118" cy="346" rx="66" ry="10" fill="rgba(43,10,13,.25)"/>
    ${extremidad([[104, 236], [80, 282], [50, 300]], 22, '#3B2A4A')}
    <ellipse cx="44" cy="304" rx="17" ry="10" fill="#2B0A0D"/>
    ${extremidad([[126, 236], [146, 288], [154, 326]], 22, '#3B2A4A')}
    <ellipse cx="166" cy="332" rx="19" ry="10" fill="#2B0A0D"/>
    ${extremidad([[92, 156], [62, 190], [56, 216]], 17, '#FFF8E7')}
    <circle cx="56" cy="220" r="11" fill="#F6C79E" stroke="#2B0A0D" stroke-width="3"/>
    <g stroke="#2B0A0D" stroke-width="3.5" stroke-linejoin="round">
      <path d="M80 146c4-10 16-14 34-14s30 4 34 14l6 84c0 10-14 16-40 16s-40-6-40-16z" fill="#FFF8E7"/>
      <path d="M84 190h60l6 42c0 8-12 13-36 13s-36-5-36-13z" fill="#DC0E20"/>
      <path d="M100 196h28v16h-28z" fill="#FFF8E7" stroke-width="2.5"/>
      <path d="M102 140l12 8 12-8v14l-12-6-12 6z" fill="#DC0E20"/>
    </g>
    ${extremidad([[142, 152], [162, 118], [168, 78]], 17, '#FFF8E7')}
    <g class="pila" stroke="#2B0A0D" stroke-width="3" stroke-linejoin="round">
      <circle cx="168" cy="72" r="11" fill="#F6C79E"/>
      <ellipse cx="166" cy="62" rx="46" ry="8" fill="#C9CED8"/>
      ${platos}
      <svg x="138" y="-34" width="58" height="58" viewBox="0 0 64 64"><use href="#${idComida('Hamburguesa')}"/></svg>
    </g>
    <g stroke="#2B0A0D" stroke-width="3.5" stroke-linejoin="round" stroke-linecap="round">
      <circle cx="114" cy="98" r="40" fill="#F6C79E"/>
      <circle cx="76" cy="104" r="9" fill="#F6C79E"/>
      <path d="M76 88c0-28 18-42 40-42s40 14 40 38c-8-6-20-8-40-8s-32 4-40 12z" fill="#DC0E20"/>
      <path d="M150 84c10-2 22 2 28 8-8 4-20 4-30 2z" fill="#A30A18"/>
      <text x="108" y="74" font-family="Impact,'Arial Black',sans-serif" font-size="20" fill="#FFF8E7" stroke="none">OS</text>
      <ellipse cx="104" cy="98" rx="7.5" ry="10" fill="#fff"/><ellipse cx="132" cy="98" rx="7.5" ry="10" fill="#fff"/>
      <circle cx="107" cy="96" r="4" fill="#2B0A0D" stroke="none"/><circle cx="135" cy="96" r="4" fill="#2B0A0D" stroke="none"/>
      <path d="M100 116q18 22 38 0z" fill="#8A1C24"/><path d="M110 124q8 5 16 0" fill="#FF7A8A" stroke-width="2"/>
      <circle cx="94" cy="114" r="6" fill="#F7A08A" stroke="none" opacity=".8"/><circle cx="144" cy="112" r="6" fill="#F7A08A" stroke="none" opacity=".8"/>
      <path d="M160 62c4 6 4 12-1 14-5-2-4-8 1-14z" fill="#7CC6F2" stroke-width="2.5"/>
    </g>`;
}
function persona(x, y, s, camisa, piel, pelo, extra) {   // Cliente de pie (pantalla de inicio)
  return `<g transform="translate(${x} ${y}) scale(${s})" stroke="#2B0A0D" stroke-width="3.5" stroke-linejoin="round" stroke-linecap="round">
    <rect x="-24" y="10" width="48" height="62" rx="20" fill="${camisa}"/>
    <rect x="-18" y="68" width="14" height="30" rx="6" fill="#3B2A4A"/><rect x="4" y="68" width="14" height="30" rx="6" fill="#3B2A4A"/>
    <circle cx="0" cy="-14" r="26" fill="${piel}"/>
    <path d="M-26 -18c0-18 12-28 26-28s26 10 26 26c-6-6-14-9-26-9s-20 4-26 11z" fill="${pelo}"/>
    ${extra}</g>`;
}
function dibujarClientesInicio() {
  const asustado = `<ellipse cx="-9" cy="-14" rx="5" ry="7" fill="#fff"/><ellipse cx="9" cy="-14" rx="5" ry="7" fill="#fff"/><circle cx="-8" cy="-13" r="2.5" fill="#2B0A0D" stroke="none"/><circle cx="10" cy="-13" r="2.5" fill="#2B0A0D" stroke="none"/><ellipse cx="0" cy="2" rx="5" ry="7" fill="#8A1C24"/>`;
  $('#clientesInicio').innerHTML =
    persona(56, 112, 1, '#8E44AD', '#C98E62', '#1E1A1A', asustado + extremidad([[-20, 26], [-34, 48], [-30, 60]], 11, '#8E44AD') + extremidad([[20, 26], [30, -4], [22, -20]], 11, '#8E44AD')) +
    persona(150, 104, 1.08, '#1D9E75', '#F6C79E', '#C98A3B', asustado + extremidad([[22, 24], [56, 8], [82, 4]], 11, '#1D9E75') + '<circle cx="86" cy="4" r="7" fill="#F6C79E"/>' +
      '<g transform="translate(30 -58)"><path d="M0 0h40a10 10 0 0 1 10 10v14a10 10 0 0 1-10 10H18l-10 10v-10H0a10 10 0 0 1-10-10V10A10 10 0 0 1 0 0z" fill="#fff"/><text x="20" y="27" font-family="Impact,sans-serif" font-size="28" fill="#DC0E20" stroke="none" text-anchor="middle">!</text></g>') +
    persona(240, 140, .78, '#FFD23F', '#FADCC0', '#6B4423', asustado);
}
function dibujarCafeFondo() {
  const ventanas = [70, 360].map(x => `<rect x="${x}" y="200" width="170" height="118" rx="8" fill="#FFE08A"/>
      <circle cx="${x + 50}" cy="262" r="18" fill="#E0A84E" stroke="none"/><rect x="${x + 28}" y="278" width="44" height="40" rx="14" fill="#E0A84E" stroke="none"/>
      <circle cx="${x + 122}" cy="258" r="16" fill="#E0A84E" stroke="none"/><rect x="${x + 102}" y="272" width="40" height="46" rx="14" fill="#E0A84E" stroke="none"/>
      <path d="M${x + 85} 200v118M${x} 250h170" fill="none"/><rect x="${x}" y="200" width="170" height="118" rx="8" fill="none"/>`).join('');
  let toldo = '';
  for (let i = 0; i < 12; i++) toldo += `<path d="M${30 + i * 45} 132h45v40a22.5 16 0 0 1-45 0z" fill="${i % 2 ? '#FFF8E7' : '#DC0E20'}"/>`;
  $('#cafeFondo').innerHTML = `<g stroke="#2B0A0D" stroke-width="5" stroke-linejoin="round">
    <rect x="40" y="70" width="520" height="340" fill="#F6E6C4"/>
    <rect x="40" y="340" width="520" height="70" fill="#B5323C"/>
    <path d="M40 365h520M40 388h520" fill="none" stroke-width="2.5" opacity=".5"/>
    <rect x="20" y="52" width="560" height="30" rx="6" fill="#A30A18"/>
    ${toldo}
    ${ventanas}
    <rect x="255" y="196" width="90" height="144" rx="6" fill="#8B5A2B"/><rect x="270" y="212" width="60" height="50" rx="4" fill="#FFE08A"/><circle cx="330" cy="282" r="5" fill="#FFD23F"/>
    <rect x="266" y="268" width="68" height="22" rx="5" fill="#4E9A2F" stroke-width="3"/>
    <text x="300" y="284" font-family="Impact,sans-serif" font-size="15" fill="#FFF8E7" stroke="none" text-anchor="middle">ABIERTO</text>
    <rect x="160" y="2" width="280" height="64" rx="14" fill="#DC0E20"/>
    <text x="300" y="49" font-family="Impact,'Arial Black',sans-serif" font-size="44" fill="#FFD23F" stroke="#2B0A0D" stroke-width="3" paint-order="stroke" text-anchor="middle" letter-spacing="2">ORDERSTACK</text>
    <path d="M60 340c-6-30 10-40 20-30 4-16 22-14 20 4 12-6 20 10 8 26z" fill="#4E9A2F"/><rect x="66" y="330" width="40" height="30" rx="4" fill="#B8743A"/>
    <path d="M494 340c-6-30 10-40 20-30 4-16 22-14 20 4 12-6 20 10 8 26z" fill="#4E9A2F"/><rect x="500" y="330" width="40" height="30" rx="4" fill="#B8743A"/>
  </g>`;
}
function dibujarRayos() {
  let s = '';
  for (let i = 0; i < 20; i++) {
    const a1 = (i / 20) * Math.PI * 2, a2 = ((i + .5) / 20) * Math.PI * 2;
    s += `<path d="M0 0L${(150 * Math.cos(a1)).toFixed(1)} ${(150 * Math.sin(a1)).toFixed(1)}L${(150 * Math.cos(a2)).toFixed(1)} ${(150 * Math.sin(a2)).toFixed(1)}z" fill="#FFE29A" opacity=".55"/>`;
  }
  $('#rayos').innerHTML = s;
}
function dibujarSilueta() {            // Ciudad de la transición día → noche (las ventanas se encienden)
  let s = '<g fill="#2B0A0D">', x = 0, k = 0;
  while (x < 1200) {
    const w = 70 + (k * 37) % 60, h = 80 + (k * 53) % 110;
    s += `<rect x="${x}" y="${220 - h}" width="${w}" height="${h}"/>`;
    for (let vy = 220 - h + 14; vy < 205; vy += 26) for (let vx = x + 12; vx < x + w - 14; vx += 22) s += `<rect class="ven" x="${vx}" y="${vy}" width="10" height="13"/>`;
    x += w + 6; k++;
  }
  s += '</g><g stroke="#2B0A0D" stroke-width="5"><rect x="470" y="96" width="260" height="124" fill="#F6E6C4"/><rect x="455" y="80" width="290" height="26" rx="6" fill="#DC0E20"/><text x="600" y="100" font-family="Impact,sans-serif" font-size="22" fill="#FFD23F" stroke="none" text-anchor="middle">ORDERSTACK</text><rect class="ven" x="495" y="128" width="80" height="60" rx="6"/><rect class="ven" x="625" y="128" width="80" height="60" rx="6"/></g>';
  $('#ctCiudad').innerHTML = s;
  let e = '';
  for (let i = 0; i < 40; i++) e += `<i style="left:${(i * 37) % 100}%;top:${(i * 23) % 55}%;animation-delay:${(i % 7) * .3}s"></i>`;
  $('#ctEstrellas').innerHTML = e;
}

// =====================================================================
// MAPA DEL RESTAURANTE (canvas, estilo caricatura con contorno grueso)
// Se dibuja a doble resolución para que se vea nítido. Solo dibuja el estado que manda C++.
// =====================================================================
const T = 48, ESCALA = 2;
const cv = $('#mapa'), g = cv.getContext('2d');
g.setTransform(ESCALA, 0, 0, ESCALA, 0, 0);
let hover = null, dirJugador = 1, ultimoMovimiento = 0;
const VENTANAS_ARRIBA = [[2, 3], [4, 5], [12, 13]];
const VENTANAS_ABAJO = [[2, 4], [10, 12]];
const ASIENTOS = [[-24, 0], [24, 0], [0, -23], [0, 23]];     // Izquierda, derecha, arriba y abajo de la mesa

function rr(x, y, w, h, r) {
  g.beginPath(); g.moveTo(x + r, y);
  g.arcTo(x + w, y, x + w, y + h, r); g.arcTo(x + w, y + h, x, y + h, r);
  g.arcTo(x, y + h, x, y, r); g.arcTo(x, y, x + w, y, r); g.closePath();
}
function trazar(relleno, grosor) {
  if (relleno) { g.fillStyle = relleno; g.fill(); }
  if (grosor !== 0) { g.lineWidth = grosor || 2.5; g.strokeStyle = TINTA; g.lineJoin = 'round'; g.lineCap = 'round'; g.stroke(); }
}
function circ(x, y, r, f, gr) { g.beginPath(); g.arc(x, y, r, 0, Math.PI * 2); trazar(f, gr); }
function elip(x, y, rx, ry, f, gr) { g.beginPath(); g.ellipse(x, y, rx, ry, 0, 0, Math.PI * 2); trazar(f, gr); }
function linea(x1, y1, x2, y2, gr, color) { g.beginPath(); g.moveTo(x1, y1); g.lineTo(x2, y2); g.lineWidth = gr; g.strokeStyle = color || TINTA; g.lineCap = 'round'; g.stroke(); }
function letrero(t, x, y, tam, color, borde) {
  g.font = `${tam}px Impact,Haettenschweiler,'Arial Black',sans-serif`; g.textAlign = 'center'; g.textBaseline = 'middle';
  if (borde) { g.lineWidth = borde; g.strokeStyle = TINTA; g.lineJoin = 'round'; g.strokeText(t, x, y); }
  g.fillStyle = color; g.fillText(t, x, y);
}
function comida(nombre, x, y, tam) { const img = IMG_COMIDA[nombre]; if (img && img.complete && img.naturalWidth) g.drawImage(img, x - tam / 2, y - tam / 2, tam, tam); }
function mezclar(a, b, k) {            // Mezcla dos colores #rrggbb
  const h = x => [1, 3, 5].map(i => parseInt(x.slice(i, i + 2), 16)), A = h(a), B = h(b);
  return '#' + A.map((v, i) => Math.round(v + (B[i] - v) * k).toString(16).padStart(2, '0')).join('');
}
function cieloDelDia(q) {              // Color del cielo según los ticks que faltan para cerrar
  if (q > 30) return {cielo: '#8EC9F0', sol: '#FFD23F', k1: 0, k2: 0};
  if (q > 10) { const k = (30 - q) / 20; return {cielo: mezclar('#8EC9F0', '#F59E5B', k), sol: mezclar('#FFD23F', '#FF7A1A', k), k1: k, k2: 0}; }
  const k = (10 - q) / 10;
  return {cielo: mezclar('#F59E5B', '#2C3263', k), sol: mezclar('#FF7A1A', '#DC0E20', k), k1: 1, k2: k};
}

// ---------- Personajes ----------
function clienteSentado(x, y, semilla, camisa, animo, t, comiendo, corona) {
  const piel = PIELES[semilla % 5], pelo = PELOS[(semilla * 3) % 5];
  const salto = animo === 2 && !comiendo ? Math.max(0, Math.sin(t / 260 + semilla)) * 1.5 : 0;
  y -= salto;
  rr(x - 10, y - 1, 20, 14, 7); trazar(camisa, 2.2);
  circ(x, y - 8, 8.5, animo === 0 ? '#F2958A' : piel, 2.2);
  g.beginPath(); g.moveTo(x - 8.5, y - 9); g.quadraticCurveTo(x - 8, y - 18, x, y - 17.5); g.quadraticCurveTo(x + 8, y - 18, x + 8.5, y - 9);
  g.quadraticCurveTo(x + 3, y - 13, x - 8.5, y - 9); trazar(pelo, 2);
  g.fillStyle = TINTA;
  if (comiendo) {                                 // Ojos cerrados de gusto y boca masticando
    g.lineWidth = 1.6; g.strokeStyle = TINTA;
    g.beginPath(); g.arc(x - 3.2, y - 7.5, 1.8, Math.PI, 0); g.stroke();
    g.beginPath(); g.arc(x + 3.2, y - 7.5, 1.8, Math.PI, 0); g.stroke();
    circ(x, y - 3.2, Math.sin(t / 90 + semilla) > 0 ? 1.8 : 1, '#8A1C24', 1);
  } else {
    circ(x - 3.2, y - 8, 1.3, TINTA, 0); circ(x + 3.2, y - 8, 1.3, TINTA, 0);
    g.lineWidth = 1.6; g.strokeStyle = TINTA; g.beginPath();
    if (animo === 2) g.arc(x, y - 5.5, 3, .15 * Math.PI, .85 * Math.PI);
    else if (animo === 1) { g.moveTo(x - 2.6, y - 3.8); g.lineTo(x + 2.6, y - 3.8); }
    else { g.arc(x, y - 1.8, 3, 1.15 * Math.PI, 1.85 * Math.PI); }
    g.stroke();
    if (animo === 0) {                            // Cejas enojadas y vapor
      linea(x - 5.5, y - 12, x - 1.5, y - 10.5, 1.6); linea(x + 5.5, y - 12, x + 1.5, y - 10.5, 1.6);
      const v = (t / 500 + semilla) % 1;
      g.globalAlpha = 1 - v; circ(x - 9, y - 18 - v * 10, 2.5 + v * 2, '#fff', 1.2); circ(x + 9, y - 18 - ((v + .5) % 1) * 10, 2 + v * 2, '#fff', 1.2); g.globalAlpha = 1;
    }
  }
  if (corona) {
    g.beginPath(); g.moveTo(x - 6, y - 16); g.lineTo(x - 6, y - 22); g.lineTo(x - 3, y - 18.5); g.lineTo(x, y - 23); g.lineTo(x + 3, y - 18.5); g.lineTo(x + 6, y - 22); g.lineTo(x + 6, y - 16); g.closePath();
    trazar('#FFD23F', 1.8);
  }
}
function clienteDePie(x, y, semilla, camisa, animo, t) {     // En la fila de la puerta
  const salto = Math.max(0, Math.sin(t / 200 + semilla)) * (animo === 0 ? 2.5 : 1);
  y -= salto;
  rr(x - 6, y + 6, 5, 9, 2); trazar('#3B2A4A', 1.8); rr(x + 1, y + 6, 5, 9, 2); trazar('#3B2A4A', 1.8);
  clienteSentado(x, y, semilla, camisa, animo, t, false, false);
}
function mesero(x, y, dir, t, carga, gorra) {    // El jugador (o el mesero de la mejora, con gorra azul)
  const cx = x + T / 2, camina = performance.now() - ultimoMovimiento < 220;
  const paso = camina ? Math.sin(t / 45) * 3 : 0, flota = camina ? 0 : Math.sin(t / 400) * 1;
  y += flota;
  elip(cx, y + T - 4, 14, 4.5, 'rgba(43,10,13,.25)', 0);
  rr(cx - 8, y + 30 + paso, 7, 12, 3); trazar('#3B2A4A', 2);
  rr(cx + 1, y + 30 - paso, 7, 12, 3); trazar('#3B2A4A', 2);
  rr(cx - 12, y + 14, 24, 21, 9); trazar('#FFF8E7', 2.5);
  rr(cx - 9.5, y + 23, 19, 12, 4); trazar(gorra === 'azul' ? '#2F7DD1' : '#DC0E20', 2);
  g.beginPath(); g.moveTo(cx - 4, y + 14); g.lineTo(cx, y + 17); g.lineTo(cx + 4, y + 14); g.lineTo(cx + 4, y + 19); g.lineTo(cx, y + 17); g.lineTo(cx - 4, y + 19); g.closePath(); trazar('#DC0E20', 1.5);
  if (carga) {                                    // Brazo arriba con la bandeja
    linea(cx + dir * 9, y + 18, cx + dir * 12, y + 2, 6.5); linea(cx + dir * 9, y + 18, cx + dir * 12, y + 2, 3.5, '#FFF8E7');
    circ(cx - dir * 13, y + 26, 3.6, '#F6C79E', 1.8);
  } else { circ(cx - 13, y + 26 + paso / 2, 3.6, '#F6C79E', 1.8); circ(cx + 13, y + 26 - paso / 2, 3.6, '#F6C79E', 1.8); }
  circ(cx, y + 7, 11.5, '#F6C79E', 2.5);
  g.beginPath(); g.arc(cx, y + 5, 11.5, Math.PI * 1.02, Math.PI * 1.98); g.closePath(); trazar(gorra === 'azul' ? '#2F7DD1' : '#DC0E20', 2.5);
  rr(cx + (dir > 0 ? 4 : -16), y + 1.5, 12, 4.5, 2); trazar(gorra === 'azul' ? '#1D5FA3' : '#A30A18', 2);
  circ(cx - 3.5 + dir * 2.5, y + 9, 1.6, TINTA, 0); circ(cx + 3.5 + dir * 2.5, y + 9, 1.6, TINTA, 0);
  g.lineWidth = 1.8; g.strokeStyle = TINTA; g.beginPath(); g.arc(cx + dir * 2, y + 12, 3.5, .15 * Math.PI, .85 * Math.PI); g.stroke();
  if (carga) {
    elip(cx + dir * 12, y - 1, 13, 4, '#C9CED8', 2);
    comida(carga, cx + dir * 12, y - 10, 22);
  }
}

// ---------- Escenario ----------
function dibujarPiso(cielo) {
  for (let f = 0; f < 10; f++) for (let c = 0; c < 15; c++) {
    g.fillStyle = (f + c) % 2 ? '#F7E7C6' : '#EDD3A3';
    g.fillRect(c * T, f * T, T, T);
  }
  if (cielo.k2 < 1) {                             // Luz de las ventanas sobre el piso
    g.fillStyle = cielo.k1 > 0 ? `rgba(255,160,80,${.28 * (1 - cielo.k2)})` : 'rgba(255,248,210,.42)';
    VENTANAS_ARRIBA.forEach(([a, b]) => {
      g.beginPath(); g.moveTo(a * T + 6, T); g.lineTo((b + 1) * T - 6, T); g.lineTo((b + 1) * T + 26, T * 3.4); g.lineTo(a * T - 8, T * 3.4); g.closePath(); g.fill();
    });
  }
}
function ventana(x, y, w, h, cielo, t, calle) {
  rr(x, y, w, h, 5); trazar(cielo.cielo, 0);
  if (cielo.k2 < .6) {                            // Nubes de día y de atardecer
    g.globalAlpha = .85 - cielo.k2;
    const nx = x + ((t / 90 + x) % (w + 30)) - 15;
    elip(nx, y + h * .35, 9, 4, '#fff', 0); elip(nx + 7, y + h * .3, 7, 4, '#fff', 0);
    g.globalAlpha = 1;
  } else {                                        // Estrellas de noche
    g.fillStyle = '#FFF8E7';
    for (let i = 0; i < 4; i++) g.fillRect(x + 6 + ((i * 17 + x) % (w - 12)), y + 5 + (i * 7) % (h * .5), 2, 2);
  }
  if (calle) { g.fillStyle = '#7E7A8C'; g.fillRect(x, y + h * .62, w, h * .38); g.fillStyle = '#F1EAD1'; for (let k = x + 6; k < x + w - 8; k += 16) g.fillRect(k, y + h * .8, 8, 2); }
  rr(x, y, w, h, 5); trazar(null, 3);
  linea(x + w / 2, y, x + w / 2, y + h, 2.5); linea(x, y + h / 2, x + w, y + h / 2, 2.5);
}
function dibujarParedes(cielo, t, cerrado) {
  g.fillStyle = '#C8202F'; g.fillRect(0, 0, 720, T);
  g.fillStyle = '#A30A18'; g.fillRect(0, T - 9, 720, 9);
  VENTANAS_ARRIBA.forEach(([a, b]) => ventana(a * T + 7, 6, (b - a + 1) * T - 14, T - 20, cielo, t, false));
  rr(8 * T + 6, 6, 2 * T + 26, T - 20, 4); trazar('#24352B', 2.5);       // Pizarra del menú
  letrero('MENÚ DEL DÍA', 9 * T + 19, 17, 11, '#F4F1E4');
  linea(8 * T + 16, 25, 10 * T + 22, 25, 1, 'rgba(244,241,228,.6)');
  g.fillStyle = '#B71C2B'; g.fillRect(0, T, T, 8 * T); g.fillRect(14 * T, T, T, 8 * T);
  g.fillStyle = '#D93342'; g.fillRect(T - 6, T, 6, 8 * T); g.fillRect(14 * T, T, 6, 8 * T);
  g.fillStyle = '#C8202F'; g.fillRect(0, 9 * T, 720, T);
  g.fillStyle = '#F1EAD1'; g.fillRect(0, 9 * T, 720, 6);
  VENTANAS_ABAJO.forEach(([a, b]) => ventana(a * T + 6, 9 * T + 12, (b - a + 1) * T - 12, T - 18, cielo, t, true));
  g.lineWidth = 3; g.strokeStyle = TINTA;
  g.strokeRect(T, T, 13 * T, 8 * T);
  // Puerta y su letrero (se voltea a CERRADO en la última llamada)
  const x = 7 * T, y = 9 * T;
  rr(x + 5, y + 4, T - 10, T - 6, 5); trazar('#8B5A2B', 2.5);
  rr(x + 10, y + 9, T - 20, 12, 3); trazar('#FFE08A', 2);
  circ(x + T - 13, y + 28, 2.5, '#FFD23F', 1.5);
  rr(x + 2, y - 10, T - 4, 14, 4); trazar(cerrado ? '#DC0E20' : '#4E9A2F', 2);
  letrero(cerrado ? 'CERRADO' : 'ABIERTO', x + T / 2, y - 2.8, 8.5, '#FFF8E7');
}
function dibujarCocina(t) {
  const x = 7 * T, y = 0, coc = E.cocina;
  rr(x + 2, y + 5, T - 4, T - 6, 6); trazar('#5B606E', 2.5);
  rr(x + 2, y + 5, T - 4, 10, 4); trazar('#7C8190', 2);
  circ(x + 15, y + 27, 7, '#2A2A30', 2); circ(x + 33, y + 27, 7, '#2A2A30', 2);
  if (coc && !coc.lista) {                        // Olla al fuego, llamas y vapor
    const fl = Math.sin(t / 70) > 0 ? '#FFD23F' : '#FF7A1A';
    circ(x + 15, y + 27, 4.5, fl, 0); circ(x + 33, y + 27, 4.5, fl, 0);
    rr(x + 9, y + 14, 30, 17, 5); trazar('#9AA3B5', 2.2);
    elip(x + 24, y + 14, 15, 3.5, '#B8BFCD', 2);
    for (let i = 0; i < 3; i++) { const v = (t / 700 + i / 3) % 1; g.globalAlpha = 1 - v; circ(x + 16 + i * 8, y + 8 - v * 14, 2.5 + v * 3, '#fff', 1.2); g.globalAlpha = 1; }
  }
}
function dibujarCaja(t, alerta) {
  const x = 11 * T, y = 0;
  if (alerta) {                                   // Anillo que late para que el jugador vaya a cobrar
    g.save(); g.setLineDash([6, 5]); g.lineDashOffset = -t / 40;
    circ(x + T / 2, y + T / 2 + 6, 28 + Math.sin(t / 150) * 3, null, 0);
    g.lineWidth = 4; g.strokeStyle = '#FFD23F'; g.stroke(); g.restore();
  }
  rr(x + 5, y + 12, T - 10, T - 16, 6); trazar('#4E9A2F', 2.5);
  rr(x + 10, y + 16, T - 20, 11, 3); trazar('#DDEFC9', 2);
  letrero('$', x + T / 2, y + 22, 11, '#2B0A0D');
  for (let k = 0; k < 3; k++) { rr(x + 11 + k * 9, y + 31, 7, 4, 1.5); trazar('#3F6B1F', 1.5); }
}
function dibujarObstaculo(n) {
  const x = n.c * T, y = n.f * T;
  elip(x + T / 2, y + T - 7, 17, 4, 'rgba(43,10,13,.25)', 0);
  rr(x + 7, y + 11, T - 14, T - 19, 4); trazar('#C98A4B', 2.5);
  rr(x + 7, y + 11, T - 14, 8, 3); trazar('#E0A66A', 2);
  g.fillStyle = '#F1EAD1'; g.fillRect(x + T / 2 - 3, y + 12, 6, T - 21);
  circ(x + T / 2 + 9, y + T / 2 + 6, 6, '#DC0E20', 1.5); letrero('J', x + T / 2 + 9, y + T / 2 + 6.5, 9, '#fff');
}
function dibujarMesa(m, t) {
  const cx = m.c * T + T / 2, cy = m.f * T + T / 2, r = m.cap === 2 ? 14 : 17;
  const asientos = m.cap === 2 ? ASIENTOS.slice(0, 2) : ASIENTOS;
  asientos.forEach(([dx, dy]) => { rr(cx + dx * .8 - 6, cy + dy * .8 - 6, 12, 12, 3); trazar('#8B5A2B', 2); });
  if (m.ocupada) {                                // Los clientes, con su ánimo según la paciencia
    const pct = m.restante / m.paciencia, animo = m.comiendo ? 2 : pct > .5 ? 2 : pct > .25 ? 1 : 0;
    const camisa = m.tipo === 0 ? ['#2E86AB', '#E4572E', '#76B041', '#A23B72', '#1D9E75', '#F28C28'][m.id % 6] : COLOR_TIPO[m.tipo];
    for (let k = 0; k < m.personas; k++) { g.save(); g.translate(cx + asientos[k][0], cy + asientos[k][1] + 2); g.scale(1.22, 1.22); clienteSentado(0, 0, m.id * 7 + k, camisa, animo, t + k * 300, m.comiendo, m.tipo === 3 && k === 0); g.restore(); }
  }
  elip(cx + 2, cy + 4, r, r * .8, 'rgba(43,10,13,.22)', 0);
  circ(cx, cy, r, '#C27B3E', 2.5);
  circ(cx - 2, cy - 2, r - 5, '#D79457', 0);
  if (m.ocupada && m.comiendo) { elip(cx, cy + 1, 9, 6, '#FFF8E7', 1.8); comida(m.plato, cx, cy - 1, 17); }
  else { circ(cx, cy, 7.5, '#FFF8E7', 1.8); letrero(m.id, cx, cy + .5, 10, TINTA); }
}

// ---------- Capa de interfaz (barras y burbujas, siempre encima) ----------
function burbujaComida(x, y, nombre, t) {
  const b = Math.sin(t / 220) * 1.2;
  rr(x, y + b, 26, 22, 8); trazar('#fff', 2.2);
  g.beginPath(); g.moveTo(x + 6, y + 21 + b); g.lineTo(x + 2, y + 28 + b); g.lineTo(x + 12, y + 21 + b); g.fillStyle = '#fff'; g.fill(); g.lineWidth = 2.2; g.strokeStyle = TINTA; g.stroke();
  comida(nombre, x + 13, y + 11 + b, 18);
}
function dibujarInterfaz(t, q) {
  E.mesas.forEach(m => {
    if (!m.ocupada || m.comiendo) return;
    const x = m.c * T, y = m.f * T, pct = limitar(m.restante / m.paciencia, 0, 1);
    rr(x + 3, y - 13, T - 6, 8, 4); trazar('#3B0F12', 2);
    if (pct > 0) { rr(x + 4.5, y - 11.5, (T - 9) * pct, 5, 2.5); trazar(pct > .5 ? '#76B041' : pct > .25 ? '#FFB020' : '#FF4655', 0); }
    burbujaComida(x + T - 8, y - 34, m.plato, t + m.id * 100);
  });
  if (E.cocina) {
    const x = 7 * T;
    if (E.cocina.lista) {
      const b = Math.sin(t / 140) * 2;
      rr(x + 6, T + 2 + b, 40, 26, 9); trazar('#FFD23F', 2.5);
      comida(E.cocina.plato, x + 18, T + 15 + b, 20); letrero('¡YA!', x + 35, T + 15 + b, 10, TINTA);
    } else { circ(x + T - 3, 6, 9, '#fff', 2); letrero(E.cocina.faltan, x + T - 3, 6.5, 11, TINTA); }
  }
  if (E.cuentas.length) {
    const x = 11 * T + T - 6, b = q <= 30 ? Math.abs(Math.sin(t / 160)) * 3 : 0;
    circ(x, 8 - b, 9, '#DC0E20', 2); letrero(E.cuentas.length, x, 8.5 - b, 11, '#fff');
  }
  E.mapa.forEach(n => { if (n.t === 6) { circ(n.c * T + 10, n.f * T + 10, 8, '#fff', 2); letrero(n.v, n.c * T + 10, n.f * T + 10.5, 10, TINTA); } });
  if (hover !== null && E.fila.length) {          // Mesa bajo el ratón: verde si cabe el grupo que sigue
    const m = E.mesas.find(mm => mm.id === hover), sig = E.fila.find(gr => gr.siguiente);
    if (m) {
      const cabe = sig && !m.ocupada && m.cap >= sig.personas;
      circ(m.c * T + T / 2, m.f * T + T / 2, 23, null, 0); g.lineWidth = 4; g.strokeStyle = cabe ? '#4E9A2F' : '#DC0E20'; g.setLineDash([5, 4]); g.stroke(); g.setLineDash([]);
    }
  }
}

function dibujarMapa(t) {
  if (!E || E.fase !== 'dia' || !$('#pDia').classList.contains('on')) return;
  const q = E.ticksDia - E.tick, cielo = cieloDelDia(q);
  g.clearRect(0, 0, 720, 480);
  dibujarPiso(cielo);
  dibujarParedes(cielo, t, E.tick >= E.ultimaLlamada);
  dibujarCocina(t);
  dibujarCaja(t, q <= 30 && E.cuentas.length > 0);
  E.mapa.forEach(n => { if (n.t === 6) dibujarObstaculo(n); });
  E.mesas.forEach(m => dibujarMesa(m, t));
  E.fila.slice(0, 4).forEach((gr, k) => {        // La fila espera en la puerta
    const pct = gr.espera / 35;
    clienteDePie(7 * T - 14 - k * 24, 9 * T + 14, gr.turno * 5, gr.tipo === 0 ? '#F28C28' : COLOR_TIPO[gr.tipo], pct > .5 ? 2 : pct > .25 ? 1 : 0, t + k * 200);
  });
  if (E.mesero) { const m = E.mesas.find(mm => mm.id === E.mesero.mesa); if (m) mesero(m.c * T + 20, (m.f - 1) * T + 6, -1, t, null, 'azul'); }
  mesero(E.jugador.c * T, E.jugador.f * T - 6, dirJugador, t, E.mano ? E.mano.plato : null);
  if (cielo.k1 > 0) {                             // Atardecer: todo se pone naranja y luego morado
    g.globalCompositeOperation = 'multiply';
    g.fillStyle = `rgba(255,170,110,${.42 * cielo.k1})`; g.fillRect(0, 0, 720, 480);
    if (cielo.k2 > 0) { g.fillStyle = `rgba(95,85,170,${.5 * cielo.k2})`; g.fillRect(0, 0, 720, 480); }
    g.globalCompositeOperation = 'source-over';
    if (cielo.k2 > 0) {                           // Se prenden las lámparas
      g.globalCompositeOperation = 'screen';
      E.mesas.forEach(m => {
        const cx = m.c * T + T / 2, cy = m.f * T + T / 2, gr = g.createRadialGradient(cx, cy, 4, cx, cy, 60);
        gr.addColorStop(0, `rgba(255,200,110,${.45 * cielo.k2})`); gr.addColorStop(1, 'rgba(255,200,110,0)');
        g.fillStyle = gr; g.fillRect(cx - 60, cy - 60, 120, 120);
      });
      g.globalCompositeOperation = 'source-over';
    }
  }
  dibujarInterfaz(t, q);
}
function animar(t) { dibujarMapa(t); requestAnimationFrame(animar); }
cv.addEventListener('mousemove', ev => {
  const r = cv.getBoundingClientRect(), c = Math.floor((ev.clientX - r.left) / r.width * 15), f = Math.floor((ev.clientY - r.top) / r.height * 10);
  const m = E && E.mesas ? E.mesas.find(mm => mm.f === f && mm.c === c) : null;
  hover = m ? m.id : null;
});
cv.addEventListener('mouseleave', () => { hover = null; });
cv.addEventListener('click', () => { if (hover !== null && !ocupado) accion('/api/sentar', {mesa: hover}); });

// =====================================================================
// DÍA: paneles, reloj con el sol, avisos de cierre y lo que se perdería al cerrar
// =====================================================================
function dibujarRelojSVG(svg, p, cielo, astro, colorBarra, marcas) {   // p = avance de 0 a 1
  const W = Math.max(240, svg.clientWidth || 600), H = 58;
  svg.setAttribute('viewBox', `0 0 ${W} ${H}`);
  const x = 22 + (W - 44) * p, y = H - 12 - (H - 28) * Math.sin(Math.PI * p);
  let s = `<rect width="${W}" height="${H}" fill="${cielo}"/>`;
  s += `<path d="M0 ${H - 10} Q${W * .2} ${H - 24} ${W * .42} ${H - 12} T${W} ${H - 16} V${H} H0z" fill="#4E9A2F" stroke="#2B0A0D" stroke-width="2.5"/>`;
  s += `<path d="M22 ${H - 12} Q${W / 2} ${-(H - 40)} ${W - 22} ${H - 12}" fill="none" stroke="#fff" stroke-opacity=".45" stroke-width="2" stroke-dasharray="4 6"/>`;
  if (astro.tipo === 'sol') {
    for (let k = 0; k < 8; k++) { const a = k * Math.PI / 4; s += `<line x1="${x + 15 * Math.cos(a)}" y1="${y + 15 * Math.sin(a)}" x2="${x + 20 * Math.cos(a)}" y2="${y + 20 * Math.sin(a)}" stroke="${astro.color}" stroke-width="3" stroke-linecap="round"/>`; }
    s += `<circle cx="${x}" cy="${y}" r="11" fill="${astro.color}" stroke="#2B0A0D" stroke-width="2.5"/>`;
  } else {
    s += `<circle cx="${x}" cy="${y}" r="11" fill="#F1EAD1" stroke="#2B0A0D" stroke-width="2.5"/><circle cx="${x + 6}" cy="${y - 4}" r="9" fill="${cielo}"/>`;
  }
  marcas.forEach(m => { s += `<line x1="${W * m}" y1="${H - 9}" x2="${W * m}" y2="${H}" stroke="#2B0A0D" stroke-width="3"/>`; });
  s += `<rect x="0" y="${H - 7}" width="${W * p}" height="7" fill="${colorBarra}"/><line x1="0" y1="${H - 7}" x2="${W}" y2="${H - 7}" stroke="#2B0A0D" stroke-width="2"/>`;
  svg.innerHTML = s;
}
function colorBarraTiempo(q) { return q > 30 ? '#76B041' : q > 15 ? '#FFB020' : q > 5 ? '#DC0E20' : '#7A0712'; }

function pasosHasta(objF, objC) {      // Pasos más cortos del jugador hasta quedar al lado de (objF, objC). Solo es una pista en pantalla
  const ocupada = {};
  E.mapa.forEach(n => { ocupada[n.f + ',' + n.c] = true; });
  const fila = [[E.jugador.f, E.jugador.c, 0]], visto = {[E.jugador.f + ',' + E.jugador.c]: true};
  for (let i = 0; i < fila.length; i++) {
    const [f, c, d] = fila[i];
    if (Math.abs(f - objF) + Math.abs(c - objC) === 1) return d;
    [[1, 0], [-1, 0], [0, 1], [0, -1]].forEach(([df, dc]) => {
      const nf = f + df, nc = c + dc, k = nf + ',' + nc;
      if (nf < 0 || nc < 0 || nf > 9 || nc > 14 || visto[k] || ocupada[k]) return;
      visto[k] = true; fila.push([nf, nc, d + 1]);
    });
  }
  return -1;
}

function dibujarDia() {
  const fila = E.fila;
  $('#filaCuenta').textContent = `${fila.length} de ${E.filaMaxima}`;
  $('#fila').innerHTML = fila.length ? fila.map(gr => {
    const pct = limitar(100 * gr.espera / 35, 0, 100), col = pct > 50 ? '#76B041' : pct > 25 ? '#FFB020' : '#FF4655';
    const animo = pct > 50 ? 2 : pct > 25 ? 1 : 0;
    let caras = ''; for (let k = 0; k < gr.personas; k++) caras += cara(gr.turno * 5 + k, animo);
    return `<div class="grupo ${gr.siguiente ? 'sig' : ''}"><div class="caras">${caras}</div>
      <div class="info"><b>${esc(gr.nombre)} · ${plural(gr.personas, 'persona', 'personas')}</b>
      <span class="tipo" style="background:${COLOR_TIPO[gr.tipo]}">${TIPOS[gr.tipo]}</span> ${gr.siguiente ? '<span class="chip am">Sigue</span>' : ''}
      <div class="pac"><i style="width:${pct}%;background:${col}"></i></div></div></div>`;
  }).join('') : '<div class="vacio">Nadie esperando.</div>';

  $('#cuentas').innerHTML = E.cuentas.length ?
    E.cuentas.map(c => `<div class="cuenta"><span>Mesa ${c.mesa}</span><b>${pesos(c.monto)}</b></div>`).join('') +
    `<div class="total"><span>Total en la caja</span><span>${pesos(E.totalCuentas)}</span></div>` :
    '<div class="vacio">No hay cuentas pendientes.</div>';

  const peds = E.pedidos.slice().sort((a, b) => a.limite - b.limite);    // Solo para mostrar en orden; la cocina saca la raíz del montículo
  $('#pedidos').innerHTML = peds.length ? peds.map((p, i) =>
    `<div class="pedido ${i === 0 ? 'raiz' : ''}">${usarComida(p.plato, 'ico g')}<div class="info"><b>${esc(p.plato)}</b> · mesa ${p.mesa}${i === 0 ? '<br><span class="chip am">Siguiente en la cocina</span>' : ''}</div><span class="t">${Math.max(0, p.restante)}</span></div>`
  ).join('') : '<div class="vacio">Sin pedidos esperando.</div>';

  $('#amenazaTxt').textContent = E.amenaza + ' / 100' + (E.jonatan.ataque ? ' · último ataque: ' + E.jonatan.ataque : '');
  $('#amenazaBarra').style.width = Math.min(100, E.amenaza) + '%';
  $('#eventos').innerHTML = E.eventos.map(e => `<div>${esc(e)}</div>`).join('');

  $('#eManoIco').innerHTML = E.mano ? usarComida(E.mano.plato, 'ico g') : '';
  $('#eMano').textContent = E.mano ? `${E.mano.plato} para la mesa ${E.mano.mesa}` : 'Nada';
  $('#eCocinaIco').innerHTML = E.cocina ? usarComida(E.cocina.plato, 'ico g') : '';
  $('#eCocina').textContent = E.cocina ? (E.cocina.lista ? `¡Listo para la mesa ${E.cocina.mesa}!` : `Mesa ${E.cocina.mesa}: faltan ${E.cocina.faltan}`) : 'Libre' + (E.mesero ? ` · mesero a la mesa ${E.mesero.mesa}` : '');
  $('#eHoy').textContent = `${E.hoy.atendidos} atendidos · ${E.hoy.perdidos} perdidos`;

  // Reloj: el sol baja y la barra cambia de color
  const q = E.ticksDia - E.tick, cielo = cieloDelDia(q);
  dibujarRelojSVG($('#relojSvg'), E.tick / E.ticksDia, cielo.cielo, {tipo: 'sol', color: cielo.sol}, colorBarraTiempo(q), [120 / 150, E.ultimaLlamada / E.ticksDia]);
  const tm = $('#tiempo');
  tm.innerHTML = `Tick ${E.tick}<small>${q > 0 ? 'quedan ' + q + ' para cerrar' : 'cerrando'}</small>`;
  tm.classList.toggle('alerta', q <= 5);
  $('#bCerrar').classList.toggle('oculto', !E.puedeCerrar);
  dibujarPerdidas(q);
}

function dibujarPerdidas(q) {          // Panel "Si el día cierra ahora pierdes" (últimos 30 ticks)
  const el = $('#perdidas');
  if (q > 30) { el.classList.add('oculto'); return; }
  el.classList.remove('oculto');
  const comiendo = E.mesas.filter(m => m.ocupada && m.comiendo), esperando = E.mesas.filter(m => m.ocupada && !m.comiendo);
  const items = [];
  if (E.cuentas.length) items.push(`<li><b>${plural(E.cuentas.length, 'cuenta', 'cuentas')} en la caja</b> (${pesos(E.totalCuentas)}) si no las cobras.</li>`);
  if (comiendo.length) items.push(`<li><b>${plural(comiendo.length, 'mesa comiendo', 'mesas comiendo')}</b> (${pesos(comiendo.reduce((s, m) => s + m.cuenta, 0))}) si no terminan antes del cierre.</li>`);
  if (esperando.length) items.push(`<li><b>${plural(esperando.length, 'mesa sin servir', 'mesas sin servir')}:</b> −${E.castigoMesa * esperando.length} de reputación si no les llega el plato.</li>`);
  const pasos = E.cuentas.length ? pasosHasta(0, 11) : -1;
  let pista = '';
  if (pasos >= 0) pista = `<div style="margin-top:4px">La caja está a <b>${plural(pasos, 'paso', 'pasos')}</b> (+1 para cobrar) y quedan <b>${q} ticks</b>.${pasos + 1 > q ? ' <b style="color:var(--rojo)">Ya no alcanzas.</b>' : ''}</div>`;
  if (!items.length) {
    el.classList.add('ok');
    el.innerHTML = `<b class="t">Todo en orden</b>Si el día cierra ahora no pierdes nada.${E.puedeCerrar ? ` Puedes cerrar temprano: los ${q} ticks que sobran pasan a la noche.` : ''}`;
  } else {
    el.classList.remove('ok');
    el.innerHTML = `<b class="t">Si el día cierra ahora pierdes</b><ul>${items.join('')}</ul>${pista}`;
  }
}

const AVISOS = [
  {q: 30, t: 'Atardece', s: () => 'Quedan 30 ticks: ve pensando en cerrar.'},
  {q: 25, t: 'Última llamada', s: () => 'Desde ahora ya no entran clientes nuevos.'},
  {q: 15, t: 'Cierre en 15', s: () => E.cuentas.length ? `Cobra las ${plural(E.cuentas.length, 'cuenta', 'cuentas')} de la caja.` : 'Termina de atender las mesas.', peligro: true},
  {q: 5, t: 'Cierre en 5', s: () => E.cuentas.length ? '¡Corre a la caja!' : 'Prepárate para la noche.', peligro: true}
];
function revisarAvisos(antes, ahora) {
  if (antes === null) return;
  const qa = E.ticksDia - antes, qn = E.ticksDia - ahora;
  AVISOS.forEach(a => {
    if (qa > a.q && qn <= a.q && qn > 0) {
      pendientes = [];                            // Se borran las teclas guardadas y se pausan un segundo: así nadie se pasa del cierre sin ver el aviso
      pausaTeclas = performance.now() + 1100;
      cartel('cartel', a.t, a.s(), a.peligro, 2400);
    }
  });
}

const DESCRIPCION_ATAQUE = {'Avalancha': 'Llegan 3 grupos de golpe.', 'Tráfico': 'Esta noche 3 calles van más lentas.', 'Caos': 'Hay cajas tiradas en el salón por 20 ticks.', 'Auditoría': 'Cada cuenta sin cobrar cuesta $5.000 y −2 de reputación.'};
function anunciarAtaque() {
  const a = E.jonatan.ataque;
  pendientes = [];
  ocupado = true;
  cutin('¡' + a.toUpperCase() + '!', DESCRIPCION_ATAQUE[a] || '', 1700).then(() => { ocupado = false; });
}

// =====================================================================
// ESCENAS: inicio de jornada, cierre del día (atardecer → noche) y fin de la noche
// =====================================================================
const RETRATO = () => `<div class="retrato" style="width:74px;height:74px"><img src="${document.querySelector('.retrato img').src}" alt=""></div>`;
function cifra(titulo, valor, clase) { return `<div class="${clase || ''}">${titulo}<b>${valor}</b></div>`; }

const QUE_PASA = [
  'Llega un grupo cada 16 ticks más o menos. Jonatan solo observa: hoy no ataca.',
  'Más clientes y más VIP. Jonatan ya puede lanzar una Avalancha, y de noche cerrar una calle.',
  'Llueve: las calles tardan 50 % más. Nuevo en el menú: Pasta y Pizza. Jonatan suma el Tráfico.',
  'Promociones: llega un grupo cada 12 ticks. Jonatan suma el Caos en el salón.',
  'La gran final: los cuatro ataques, 5 domicilios y Jonatan puede cerrar 2 calles.'
];
async function escenaJornada() {
  await abrirEscena(`<h2>Jornada ${E.jornada}: ${esc(E.nombreJornada)}</h2>
    <div class="jonatan" style="margin:8px 0 12px">${RETRATO()}<div class="burbuja">«${esc(E.jonatan.frase)}»</div></div>
    <p class="sub" style="font-size:14px">${QUE_PASA[E.jornada - 1]}</p>`,
    [{texto: '¡Abrir el restaurante!', clase: 'am', valor: 1}]);
}

async function cieloAnochece() {      // Animación: el sol se oculta, sale la luna y se prenden las ventanas
  const c = $('#cieloT');
  c.classList.remove('anochece');
  c.classList.add('on');
  await espera(350);
  c.classList.add('anochece');
  await espera(2300);
}
function cerrarCielo() { const c = $('#cieloT'); c.classList.remove('on'); setTimeout(() => c.classList.remove('anochece'), 600); }

async function escenaCierreDia(perdio) {
  ocupado = true;
  pendientes = [];
  const h = E.hoy, cambio = h.repFinDia - h.repInicio;
  if (perdio) {
    await abrirEscena(`<h2>Jonatan se levantó de la mesa</h2>
      <div class="jonatan" style="margin:8px 0 12px">${RETRATO()}<div class="burbuja">«${esc(E.jonatan.frase)}»</div></div>
      <p class="sub" style="font-size:14px">La reputación llegó a 0 en la jornada ${E.jornada}. El restaurante cierra y Jonatan escribe su reseña.</p>`,
      [{texto: 'Ver la reseña', clase: 'ro', valor: 1}]);
    return;
  }
  await cieloAnochece();
  const html = `<h2>OrderStack cerró por hoy</h2>
    <div class="cifras">
      ${cifra('Ventas en la caja', pesos(h.ventas))}
      ${cifra('Reputación', `${h.repInicio} → ${h.repFinDia}`, cambio >= 0 ? 'bien' : 'mal')}
      ${cifra('Clientes atendidos', h.atendidos, 'bien')}
      ${cifra('Se fueron sin comer', h.perdidos, h.perdidos ? 'mal' : '')}
    </div>
    ${h.sinCobrar ? `<p class="sub" style="font-size:14px">Se perdieron <b>${pesos(h.sinCobrar)}</b> en cuentas sin cobrar.</p>` : '<p class="sub" style="font-size:14px">No se perdió ninguna cuenta. ¡Bien cobrado!</p>'}
    <div class="jonatan" style="margin:10px 0 4px">${RETRATO()}<div class="burbuja">«${esc(E.jonatan.frase)}»</div></div>
    <p class="sub" style="font-size:14px">Esta noche: <b>${plural(E.noche.domicilios.length, 'domicilio', 'domicilios')}</b> y <b>${E.noche.duracion} ticks</b>.</p>`;
  const p = abrirEscena(html, [{texto: 'Empezar la noche', clase: 'am', valor: 1}]);
  cerrarCielo();
  await p;
}

async function escenaFinNoche(esFinal) {
  ocupado = true;
  pendientes = [];
  const ultima = E.historial[E.historial.length - 1];
  const terminoNoche = ultima && ultima.jornada === E.jornada && (E.fase !== 'final' || E.estrellas >= 3);
  if (!terminoNoche) {                            // Perdió por reputación en plena noche
    await abrirEscena(`<h2>Jonatan cerró su libreta</h2>
      <div class="jonatan" style="margin:8px 0 12px">${RETRATO()}<div class="burbuja">«${esc(E.jonatan.frase)}»</div></div>
      <p class="sub" style="font-size:14px">La reputación llegó a 0 en la noche ${E.jornada}. Se acabó la partida.</p>`,
      [{texto: 'Ver la reseña', clase: 'ro', valor: 1}]);
    return;
  }
  const h = ultima, sinEntregar = h.pedidos - h.entregas, tarde = h.entregas - h.aTiempo;
  const html = `<h2>${esFinal ? 'Se acabó la última noche' : 'Noche terminada'}</h2>
    <div class="cifras">
      ${cifra('Llegaron calientes', `${h.aTiempo} de ${h.pedidos}`, h.aTiempo === h.pedidos ? 'bien' : '')}
      ${cifra('Llegaron fríos', tarde, tarde ? 'mal' : '')}
      ${cifra('Sin entregar', sinEntregar, sinEntregar ? 'mal' : '')}
      ${cifra('Ganado de noche', pesos(h.noche))}
      ${cifra('Rutas perfectas', `${h.rutasPerfectas} de ${h.viajes}`, h.rutasPerfectas ? 'bien' : '')}
      ${cifra('Reputación', h.reputacion, h.reputacion >= E.metaRep ? 'bien' : 'mal')}
    </div>
    ${h.aTiempo === h.pedidos && h.pedidos > 0 ? '<p class="sub" style="font-size:14px"><b>¡Noche perfecta!</b> +50 puntos en el ranking.</p>' : ''}
    ${h.cierres ? `<p class="sub" style="font-size:14px">Jonatan cerró ${plural(h.cierres, 'calle', 'calles')} esta noche.</p>` : ''}
    <div class="jonatan" style="margin:10px 0 4px">${RETRATO()}<div class="burbuja">«${esc(esFinal ? 'Ya tengo todo lo que necesito para mi reseña.' : E.jonatan.frase)}»</div></div>`;
  await abrirEscena(html, [{texto: esFinal ? 'Ver la reseña de Jonatan' : 'Ir a la tienda', clase: esFinal ? 'ro' : 'am', valor: 1}]);
}

// =====================================================================
// NOCHE: el jugador traza su ruta sobre el grafo de la ciudad.
// Las distancias del GPS vienen de C++ (Dijkstra desde cada lugar); la página solo las muestra.
// =====================================================================
const POS = [[380, 64], [230, 160], [530, 160], [230, 300], [530, 300], [80, 120], [80, 245], [680, 110], [680, 245], [80, 400], [380, 412]];
const LETRA = ['', 'A', 'B', 'C', 'D'];
let seleccion = [];                    // Pedidos elegidos para este viaje
let traza = [];                        // Lugares de la ruta trazada (empieza donde está la moto)
let animandoViaje = false, ofertaAbierta = false, avisoSinTiempo = false;

function reiniciarNoche() { seleccion = []; traza = [E.noche.pos]; avisoSinTiempo = false; }
const nombreCorto = n => n === 0 ? 'Restaurante' : n <= 4 ? 'Esquina ' + LETRA[n] : E.noche.nodos[n];
function calleEntre(a, b) { return E.noche.calles.find(c => (c.a === a && c.b === b) || (c.a === b && c.b === a)); }
function abierta(a, b) { const c = calleEntre(a, b); return c && !c.cerrada ? c : null; }
function costoTraza(r) { let s = 0; for (let i = 0; i + 1 < r.length; i++) { const c = abierta(r[i], r[i + 1]); if (!c) return -1; s += c.costo; } return s; }
function vecinosAbiertos(n) { return E.noche.calles.filter(c => !c.cerrada && (c.a === n || c.b === n)).map(c => c.a === n ? c.b : c.a); }
const enDesvio = () => E.noche.estado === 'desvio';
function pedidosObjetivo() {           // Pedidos por los que debe pasar la ruta
  if (enDesvio()) return E.noche.domicilios.filter(d => d.enMoto && d.estado === 0);
  return seleccion.map(id => E.noche.domicilios.find(d => d.id === id)).filter(Boolean);
}
function mejorTiempo(desde, casas) {   // Lo que tarda la ruta del GPS (mismas cuentas que C++, con sus distancias de Dijkstra)
  const D = E.noche.dist;
  if (!D.length) return -1;
  if (casas.length === 0) return D[desde][0];
  if (casas.length === 1) return D[desde][casas[0]] + D[casas[0]][0];
  const [a, b] = casas;
  return Math.min(D[desde][a] + D[a][b] + D[b][0], D[desde][b] + D[b][a] + D[a][0]);
}
function colorBarraNoche(q) { return q > 20 ? '#76B041' : q > 10 ? '#FFB020' : q > 5 ? '#DC0E20' : '#7A0712'; }

// ---------- Mapa de la ciudad (SVG) ----------
const ESTRELLAS_CIUDAD = Array.from({length: 46}, (_, i) => [(i * 97) % 760, (i * 53) % 500, i % 3]);
function dibujarCiudad() {
  const N = E.noche, ultimo = traza[traza.length - 1], libre = !animandoViaje && !ocupado && E.fase === 'noche';
  const vecinos = libre ? vecinosAbiertos(ultimo) : [];
  const enTraza = new Set();
  for (let i = 0; i + 1 < traza.length; i++) enTraza.add(Math.min(traza[i], traza[i + 1]) + '-' + Math.max(traza[i], traza[i + 1]));
  let s = '<rect width="760" height="500" fill="#1E2340"/>';
  ESTRELLAS_CIUDAD.forEach(([x, y, k]) => { s += `<circle class="${k ? 'titila' : ''}" style="animation-delay:${(x % 9) * .25}s" cx="${x}" cy="${y}" r="${k === 2 ? 1.8 : 1.2}" fill="#FFF8E7" opacity=".7"/>`; });
  s += `<rect x="262" y="192" width="236" height="76" rx="18" fill="#2E5A3A" stroke="#0D0F22" stroke-width="3"/>`;          // Parque del centro
  [[300, 222], [345, 238], [410, 220], [455, 240], [380, 248]].forEach(([x, y]) => { s += `<circle cx="${x}" cy="${y}" r="13" fill="#3F7A4E" stroke="#0D0F22" stroke-width="2.5"/>`; });
  [[120, 20, 80, 40], [570, 20, 70, 44], [600, 330, 110, 50], [130, 320, 60, 50], [455, 360, 90, 40]].forEach(([x, y, w, h]) => {
    s += `<rect x="${x}" y="${y}" width="${w}" height="${h}" rx="8" fill="#262B4D" stroke="#0D0F22" stroke-width="2.5"/>`;
    for (let vx = x + 10; vx < x + w - 10; vx += 16) s += `<rect x="${vx}" y="${y + 12}" width="7" height="9" fill="#FFD23F" opacity=".55"/>`;
  });
  N.calles.forEach(c => {                         // Calles: contorno, asfalto y raya del centro
    const [x1, y1] = POS[c.a], [x2, y2] = POS[c.b];
    const asfalto = c.cerrada ? '#3A3A44' : c.lenta ? '#7A3550' : '#4A5080';
    s += `<line x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" stroke="#0D0F22" stroke-width="28" stroke-linecap="round"/>`;
    s += `<line x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" stroke="${asfalto}" stroke-width="21" stroke-linecap="round"/>`;
    if (!c.cerrada) s += `<line x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" stroke="#F1EAD1" stroke-opacity=".5" stroke-width="2" stroke-dasharray="9 9"/>`;
    const esVecina = vecinos.length && ((c.a === ultimo && vecinos.includes(c.b)) || (c.b === ultimo && vecinos.includes(c.a)));
    if (esVecina) s += `<line x1="${x1}" y1="${y1}" x2="${x2}" y2="${y2}" stroke="#FFD23F" stroke-opacity=".35" stroke-width="21" stroke-linecap="round"/>`;
  });
  if (N.gps.length > 1 && !animandoViaje) s += `<polyline points="${N.gps.map(i => POS[i].join(',')).join(' ')}" fill="none" stroke="#5EE0FF" stroke-width="5" stroke-dasharray="3 10" stroke-linecap="round" stroke-linejoin="round"/>`;
  if (traza.length > 1 && !animandoViaje) {       // Ruta trazada por el jugador, con flechas
    const pts = traza.map(i => POS[i].join(',')).join(' ');
    s += `<polyline points="${pts}" fill="none" stroke="#0D0F22" stroke-width="14" stroke-linejoin="round" stroke-linecap="round"/>`;
    s += `<polyline points="${pts}" fill="none" stroke="#FFD23F" stroke-width="8" stroke-linejoin="round" stroke-linecap="round"/>`;
    for (let i = 0; i + 1 < traza.length; i++) {
      const [x1, y1] = POS[traza[i]], [x2, y2] = POS[traza[i + 1]], a = Math.atan2(y2 - y1, x2 - x1) * 180 / Math.PI;
      const mx = x1 + (x2 - x1) * .62, my = y1 + (y2 - y1) * .62;
      s += `<path d="M-7 -7L7 0L-7 7z" transform="translate(${mx} ${my}) rotate(${a})" fill="#DC0E20" stroke="#0D0F22" stroke-width="2" stroke-linejoin="round"/>`;
    }
  }
  N.calles.forEach((c, i) => {                    // Costo de cada calle (o la barrera si está cerrada)
    const [x1, y1] = POS[c.a], [x2, y2] = POS[c.b], mx = (x1 + x2) / 2, my = (y1 + y2) / 2, a = Math.atan2(y2 - y1, x2 - x1) * 180 / Math.PI;
    if (c.cerrada) {
      s += `<g transform="translate(${mx} ${my}) rotate(${a + 90})"><rect x="-24" y="-8" width="48" height="16" rx="4" fill="#fff" stroke="#0D0F22" stroke-width="3"/>`;
      for (let k = -18; k < 24; k += 12) s += `<path d="M${k} -6l6 0l-6 12l-6 0z" fill="#DC0E20"/>`;
      s += `</g>${i === N.calleCerrada ? `<circle cx="${mx}" cy="${my}" r="30" fill="none" stroke="#FF4655" stroke-width="4" class="anillo"/>` : ''}`;
      return;
    }
    if (c.lenta) s += `<path d="M${mx - 26} ${my + 12}l6-16 6 16z" fill="#FF7A1A" stroke="#0D0F22" stroke-width="2"/>`;
    s += `<circle cx="${mx}" cy="${my}" r="14" fill="${c.lenta ? '#FFB3A8' : '#F1EAD1'}" stroke="#0D0F22" stroke-width="3"/><text x="${mx}" y="${my + 6}" text-anchor="middle" font-family="Impact,sans-serif" font-size="17" fill="#2B0A0D">${c.costo}</text>`;
  });
  N.nodos.forEach((nombre, i) => {               // Lugares: restaurante, esquinas y casas
    const [x, y] = POS[i], clic = libre && (vecinos.includes(i) || (i === ultimo && traza.length > 1));
    s += `<g class="nodo ${clic ? 'clic' : ''}" data-nodo="${i}"><title>${esc(nombreCorto(i))}</title>`;
    if (clic && i !== ultimo) s += `<circle cx="${x}" cy="${y}" r="${i === 0 ? 40 : i <= 4 ? 25 : 36}" fill="#FFD23F" fill-opacity=".25" stroke="#FFD23F" stroke-width="3" class="anillo"/>`;
    if (i === 0) {
      s += `<rect x="${x - 62}" y="${y - 24}" width="124" height="46" rx="6" fill="#F6E6C4" stroke="#0D0F22" stroke-width="3"/>`;
      for (let k = 0; k < 6; k++) s += `<path d="M${x - 62 + k * 20.6} ${y - 24}h20.6v10a10.3 7 0 0 1-20.6 0z" fill="${k % 2 ? '#FFF8E7' : '#DC0E20'}" stroke="#0D0F22" stroke-width="2"/>`;
      s += `<rect x="${x - 44}" y="${y - 4}" width="30" height="18" rx="3" fill="#FFD23F" stroke="#0D0F22" stroke-width="2"/><rect x="${x + 14}" y="${y - 4}" width="30" height="18" rx="3" fill="#FFD23F" stroke="#0D0F22" stroke-width="2"/>`;
      s += `<rect x="${x - 56}" y="${y - 48}" width="112" height="22" rx="6" fill="#DC0E20" stroke="#0D0F22" stroke-width="3"/><text x="${x}" y="${y - 31.5}" text-anchor="middle" font-family="Impact,'Arial Narrow',sans-serif" font-size="14" fill="#FFD23F" textLength="96" lengthAdjust="spacingAndGlyphs">ORDERSTACK</text>`;
    } else if (i <= 4) {
      s += `<circle cx="${x}" cy="${y}" r="17" fill="#3B4170" stroke="#0D0F22" stroke-width="3"/><text x="${x}" y="${y + 6}" text-anchor="middle" font-family="Impact,sans-serif" font-size="17" fill="#F1EAD1">${LETRA[i]}</text>`;
    } else {
      const d = N.domicilios.find(dd => dd.casa === i), sel = d && (seleccion.includes(d.id) || d.enMoto);
      const techo = !d ? '#6C7AA8' : d.estado === 1 ? '#76B041' : d.estado === 2 ? '#5EA9E0' : sel ? '#FFD23F' : d.urgente ? '#FF4655' : '#FCB14E';
      s += `<path d="M${x - 30} ${y - 4}L${x} ${y - 30}L${x + 30} ${y - 4}z" fill="${techo}" stroke="#0D0F22" stroke-width="3" stroke-linejoin="round"/>`;
      s += `<rect x="${x - 23}" y="${y - 4}" width="46" height="30" fill="#F1EAD1" stroke="#0D0F22" stroke-width="3"/><rect x="${x - 6}" y="${y + 8}" width="12" height="18" fill="#8B5A2B" stroke="#0D0F22" stroke-width="2"/><rect x="${x + 9}" y="${y + 2}" width="9" height="9" fill="#FFD23F" stroke="#0D0F22" stroke-width="2"/>`;
      s += `<text x="${x}" y="${y + 44}" text-anchor="middle" font-size="12.5" font-weight="700" fill="#E6E8FF">${esc(nombre)}</text>`;
      if (d) {
        const r = d.limite - N.tick;
        s += `<circle cx="${x + 27}" cy="${y - 27}" r="13" fill="${d.urgente ? '#FF4655' : '#FFF8E7'}" stroke="#0D0F22" stroke-width="2.5"/><text x="${x + 27}" y="${y - 22}" text-anchor="middle" font-family="Impact,sans-serif" font-size="15" fill="#2B0A0D">${d.estado === 1 ? '✓' : d.estado === 2 ? '!' : d.id}</text>`;
        if (d.estado === 0) s += `<rect x="${x - 34}" y="${y + 50}" width="68" height="18" rx="9" fill="${r < 0 ? '#5EA9E0' : r <= 8 ? '#FF4655' : '#FFF8E7'}" stroke="#0D0F22" stroke-width="2"/><text x="${x}" y="${y + 63.5}" text-anchor="middle" font-size="11.5" font-weight="800" fill="#2B0A0D">${r < 0 ? 'frío' : 'quedan ' + r}</text>`;
      }
    }
    s += '</g>';
  });
  const [mx, my] = POS[N.pos];
  s += `<g id="motoG" transform="translate(${mx + (N.pos === 0 ? 96 : 0)} ${my + (N.pos === 0 ? 6 : -14)})">${MOTO}</g>`;
  const svg = $('#ciudad');
  svg.innerHTML = s;
  svg.querySelectorAll('.nodo.clic').forEach(el => el.addEventListener('click', () => clicNodo(+el.dataset.nodo)));
}

function clicNodo(n) {
  if (ocupado || animandoViaje) return;
  const ultimo = traza[traza.length - 1];
  if (n === ultimo && traza.length > 1) { traza.pop(); dibujarNoche(); return; }
  if (!abierta(ultimo, n) || traza.length >= 40) return;
  traza.push(n);
  if (!enDesvio() && n >= 5) {                    // Si llega a una casa con pedido y hay cupo en la moto, lo elige solo
    const d = E.noche.domicilios.find(dd => dd.casa === n && dd.estado === 0);
    if (d && !seleccion.includes(d.id) && seleccion.length < E.noche.capacidad) seleccion.push(d.id);
  }
  dibujarNoche();
}
function elegirPedido(id) {
  if (ocupado || animandoViaje || enDesvio()) return;
  if (seleccion.includes(id)) seleccion = seleccion.filter(x => x !== id);
  else { seleccion.push(id); if (seleccion.length > E.noche.capacidad) seleccion.shift(); }
  dibujarNoche();
}
function completarRegreso() {          // Sigue el camino más corto de vuelta (con las distancias de Dijkstra que mandó C++)
  const D = E.noche.dist;
  let n = traza[traza.length - 1], vueltas = 0;
  while (n !== 0 && vueltas++ < 20) {
    const sig = vecinosAbiertos(n).find(v => abierta(n, v).costo + D[v][0] === D[n][0]);
    if (sig === undefined) break;
    traza.push(sig); n = sig;
  }
  dibujarNoche();
}

// ---------- Paneles de la noche ----------
function dibujarNoche() {
  const N = E.noche;
  if (!animandoViaje && (!traza.length || traza[0] !== N.pos)) traza = [N.pos];
  const pend = N.domicilios.filter(d => d.estado === 0);
  if (enDesvio()) seleccion = N.domicilios.filter(d => d.enMoto && d.estado === 0).map(d => d.id);
  else seleccion = seleccion.filter(id => pend.some(d => d.id === id)).slice(0, N.capacidad);
  dibujarCiudad();

  const q = N.duracion - N.tick;
  dibujarRelojSVG($('#relojNocheSvg'), limitar(N.tick / N.duracion, 0, 1), '#2C3263', {tipo: 'luna'}, colorBarraNoche(q), []);
  $('#tiempoNoche').innerHTML = `Noche ${N.tick}<small>${q > 0 ? 'quedan ' + q + ' de ' + N.duracion : 'se acabó el tiempo'}</small>`;
  $('#tiempoNoche').classList.toggle('alerta', q <= 5);
  $('#motoTxt').textContent = N.capacidad === 2 ? 'moto mejorada: hasta 2 por viaje' : '1 pedido por viaje';
  const dz = $('#desvio');
  dz.classList.toggle('oculto', !enDesvio());
  if (enDesvio()) dz.innerHTML = `¡Jonatan cerró una calle! La moto está en <b>${esc(nombreCorto(N.pos))}</b>. Traza un desvío desde ahí hasta las casas que faltan y de vuelta al restaurante.`;

  $('#domicilios').innerHTML = N.domicilios.map(d => {
    const r = d.limite - N.tick, pct = limitar(r / 45, 0, 1), sel = seleccion.includes(d.id), ida = N.dist.length ? N.dist[0][d.casa] : 0;
    const color = r < 0 ? '#5EA9E0' : r <= 8 ? '#FF4655' : r <= 18 ? '#FFB020' : '#FF7A1A';
    let estado;
    if (d.estado === 1) estado = `<span class="chip ve">Llegó caliente en el tick ${d.llegada}: ${pesos(d.pago)}</span>`;
    else if (d.estado === 2) estado = `<span class="chip az">Llegó frío en el tick ${d.llegada}: ${pesos(d.pago)}</span>`;
    else estado = `<div class="termo"><span>${r < 0 ? 'Ya está frío' : 'Caliente'}</span><div class="tubo"><i style="width:${r < 0 ? 100 : pct * 100}%;background:${color}"></i></div><span>límite: tick ${d.limite}</span></div>
      <span style="font-size:12px">GPS: ida más corta ${ida} ticks${d.enMoto ? ' · <b>va en la moto</b>' : ''}</span>`;
    return `<div class="dom ${sel ? 'sel' : ''} ${d.estado ? 'hecho' : ''} ${d.urgente && !d.estado ? 'urg' : ''}" data-id="${d.id}"><div class="num">${d.id}</div>
      <div class="info"><b>${esc(d.nombreCasa)}</b> ${d.urgente ? '<span class="chip ro">Urgente: paga el doble</span>' : ''}<br>${estado}</div></div>`;
  }).join('') || '<div class="vacio">No hay pedidos.</div>';
  document.querySelectorAll('.dom').forEach(el => { if (!el.classList.contains('hecho')) el.addEventListener('click', () => elegirPedido(+el.dataset.id)); });
  dibujarPanelRuta();
  dibujarFinNoche(q, pend);
  if (N.oferta && !ofertaAbierta && !animandoViaje && !ocupado) { ofertaAbierta = true; escenaUrgente().then(() => { ofertaAbierta = false; }); }
}

function dibujarPanelRuta() {
  const N = E.noche, objetivo = pedidosObjetivo(), desde = traza[0], desvio = enDesvio();
  $('#rutaTxt').textContent = desvio ? 'desvío' : `${seleccion.length} de ${N.capacidad} pedidos`;
  const bS = $('#bSalir');
  bS.textContent = desvio ? 'Seguir el viaje' : 'Salir con la moto';
  if (!desvio && !objetivo.length) {
    $('#ruta').innerHTML = `<p class="sub" style="font-size:14px;margin:0">1. Elige ${N.capacidad === 2 ? 'uno o dos pedidos' : 'un pedido'} en la lista (o haz clic en su casa).<br>2. Traza la ruta haciendo clic en las esquinas iluminadas.<br>3. Vuelve al restaurante y pulsa <b>Salir con la moto</b>.</p>`;
    bS.disabled = true; $('#bRegreso').disabled = true; $('#bGps').disabled = true; $('#bDeshacer').disabled = traza.length < 2; $('#bBorrar').disabled = traza.length < 2;
    return;
  }
  const costo = costoTraza(traza), mejor = mejorTiempo(desde, objetivo.map(d => d.casa));
  const visitadas = objetivo.every(d => traza.indexOf(d.casa, 1) > 0);
  const completa = traza.length > 1 && traza[traza.length - 1] === 0 && visitadas;
  const vuelta = N.tick + costo, cabe = desvio || vuelta <= N.duracion;
  let html = `<div class="pasos">${traza.map((n, i) => `${i ? '<i>›</i>' : ''}<span class="${n >= 5 ? 'casa' : ''}">${esc(nombreCorto(n))}</span>`).join('')}</div>`;
  html += `<div class="comparar"><div class="${completa && costo === mejor ? 'igual' : ''}">Tu ruta<b>${costo} ticks</b></div><div>GPS (Dijkstra)<b>${mejor} ticks</b></div></div>`;
  html += objetivo.map(d => {
    const k = traza.indexOf(d.casa, 1);
    if (k < 0) return `<div style="font-size:13px">Pedido ${d.id} (${esc(d.nombreCasa)}): tu ruta todavía no pasa por ahí.</div>`;
    const llega = N.tick + costoTraza(traza.slice(0, k + 1)), ok = llega <= d.limite;
    return `<div style="font-size:13px">Pedido ${d.id} (${esc(d.nombreCasa)}): llega en el tick <b>${llega}</b>, límite ${d.limite}: <b style="color:${ok ? 'var(--verde)' : '#2F7DD1'}">${ok ? 'caliente' : 'frío'}</b></div>`;
  }).join('');
  if (completa) html += `<div style="font-size:13px;margin-top:4px">Vuelves en el tick <b>${vuelta}</b> de ${N.duracion}.${cabe ? '' : ' <b style="color:var(--rojo)">No alcanzas: acorta la ruta o lleva otro pedido.</b>'}</div>`;
  else if (visitadas) html += `<div style="font-size:13px;margin-top:4px">Falta volver al restaurante: pulsa <b>Completar regreso</b> o sigue trazando.</div>`;
  if (N.gpsVisto) html += `<div style="margin-top:6px"><span class="chip az">Miraste el GPS: este viaje no da bono</span></div>`;
  else if (completa && costo === mejor) html += `<div style="margin-top:6px"><span class="chip am">¡Igual al GPS! Bono de ${pesos(N.bonoRuta)} al volver</span></div>`;
  $('#ruta').innerHTML = html;
  bS.disabled = !(completa && cabe);
  $('#bRegreso').disabled = !visitadas || traza[traza.length - 1] === 0;
  $('#bGps').disabled = false;
  $('#bDeshacer').disabled = traza.length < 2;
  $('#bBorrar').disabled = traza.length < 2;
}

function dibujarFinNoche(q, pend) {
  const N = E.noche, desvio = enDesvio();
  const alcanza = desvio || pend.some(d => N.dist.length && N.tick + N.dist[0][d.casa] * 2 <= N.duracion);
  const castigo = pend.length * N.castigoSinEntregar, queda = Math.max(0, E.reputacion - castigo);
  let html;
  if (!alcanza && pend.length) {
    html = `<div class="perdidas"><b class="t">Ya no alcanzas a salir</b>Ningún pedido alcanza ida y vuelta en los ${Math.max(0, q)} ticks que quedan.<br>
      Quedan ${plural(pend.length, 'pedido', 'pedidos')} sin entregar: <b>−${castigo} de reputación</b> (${E.reputacion} → ${queda}).${queda <= 0 ? '<br><b style="color:var(--rojo)">¡Con esto pierdes la partida!</b>' : ''}</div>
      <div class="acciones"><button class="btn ro" id="bTerminar">Terminar la noche</button></div>`;
    if (!avisoSinTiempo) { avisoSinTiempo = true; cartel('cartelNoche', 'Se acabó el tiempo', 'Ya no alcanzas a salir a ningún domicilio.', true, 2600); }
  } else {
    html = `<p class="sub" style="margin:0 0 6px">La noche termina sola cuando entregas todo. Si la terminas antes, cada pedido sin entregar resta ${N.castigoSinEntregar} de reputación${pend.length ? ` (ahora: −${castigo})` : ''}.</p>
      <div class="acciones"><button class="btn cr chico" id="bTerminar" ${desvio ? 'disabled title="Primero vuelve con el desvío"' : ''}>Terminar la noche</button></div>`;
  }
  $('#finNoche').innerHTML = html;
  $('#bTerminar').addEventListener('click', terminarNocheConfirmar);
}

async function terminarNocheConfirmar() {
  if (ocupado || animandoViaje) return;
  const pend = E.noche.domicilios.filter(d => d.estado === 0);
  if (!pend.length) { accion('/api/terminarNoche'); return; }
  const castigo = pend.length * E.noche.castigoSinEntregar, queda = Math.max(0, E.reputacion - castigo);
  const v = await abrirEscena(`<h2>¿Terminar la noche?</h2>
    <p>Quedan ${plural(pend.length, 'pedido', 'pedidos')} sin entregar: ${pend.map(d => esc(d.nombreCasa)).join(', ')}.</p>
    <div class="cifras">${cifra('Castigo', '−' + castigo, 'mal')}${cifra('Reputación', E.reputacion + ' → ' + queda, queda <= E.metaRep ? 'mal' : '')}</div>
    ${queda <= 0 ? '<p style="color:var(--rojo);font-weight:800">¡Con esto la reputación llega a 0 y pierdes la partida!</p>' : ''}`,
    [{texto: 'Seguir repartiendo', clase: 'cr', valor: 0}, {texto: 'Terminar la noche', clase: 'ro', valor: 1}]);
  if (v) accion('/api/terminarNoche');
}

async function verGps() {
  if (ocupado || animandoViaje) return;
  if (!enDesvio() && !seleccion.length) { avisar('Elige primero un pedido.', false); return; }
  if (!E.noche.gpsVisto) {
    const v = await abrirEscena(`<h2>¿Mirar el GPS?</h2><p>El GPS te muestra la ruta más rápida (calculada con Dijkstra), pero <b>este viaje ya no dará el bono de ${pesos(E.noche.bonoRuta)}</b>.</p>`,
      [{texto: 'Mejor la busco yo', clase: 'cr', valor: 0}, {texto: 'Mostrar la ruta', clase: 'am', valor: 1}]);
    if (!v) return;
  }
  const j = await accion('/api/ruta', {a: seleccion[0] || -1, b: seleccion[1] || -1});
  if (j && j.ok) { traza = E.noche.gps.slice(); dibujarNoche(); }
}

// ---------- El viaje: la moto recorre lo que C++ dice que recorrió ----------
function moverMoto(el, p1, p2, dur) {
  return new Promise(res => {
    const ini = performance.now(), izq = p2[0] < p1[0];
    function paso(ahora) {
      const k = Math.min(1, (ahora - ini) / dur), e = k < .5 ? 2 * k * k : 1 - Math.pow(-2 * k + 2, 2) / 2;
      const x = p1[0] + (p2[0] - p1[0]) * e, y = p1[1] + (p2[1] - p1[1]) * e - 14 - Math.abs(Math.sin(k * Math.PI * 4)) * 2;
      el.setAttribute('transform', `translate(${x} ${y}) scale(${izq ? -1 : 1} 1)`);
      if (k < 1) requestAnimationFrame(paso); else res();
    }
    requestAnimationFrame(paso);
  });
}
function globo(p, texto, color) {      // Texto que sube desde una casa (+$ o ¡frío!)
  const svg = $('#ciudad'), el = document.createElementNS('http://www.w3.org/2000/svg', 'g');
  el.setAttribute('class', 'globo');
  el.innerHTML = `<text x="${p[0]}" y="${p[1] - 40}" text-anchor="middle" font-family="Impact,sans-serif" font-size="26" fill="${color}" stroke="#0D0F22" stroke-width="5" paint-order="stroke">${texto}</text>`;
  svg.appendChild(el);
  setTimeout(() => el.remove(), 1500);
}
async function animarViaje(rec, entregas, nuevo) {
  const moto = $('#motoG');
  if (!moto || rec.length < 2) return;
  for (let i = 0; i + 1 < rec.length; i++) {
    const c = calleEntre(rec[i], rec[i + 1]);
    await moverMoto(moto, POS[rec[i]], POS[rec[i + 1]], 180 + 95 * (c ? c.costo : 1));
    entregas.filter(en => en.paso === i + 1).forEach(en => {
      const d = nuevo.noche.domicilios.find(dd => dd.id === en.id);
      if (d) globo(POS[rec[i + 1]], d.estado === 1 ? '+' + pesos(d.pago) : '¡Frío! +' + pesos(d.pago), d.estado === 1 ? '#FFD23F' : '#5EE0FF');
    });
  }
}
async function salirMoto() {
  if (ocupado || animandoViaje || enviando || $('#bSalir').disabled) return;
  const desvio = enDesvio(), antes = E;
  enviando = true;
  let j;
  try { j = await api(desvio ? '/api/desviar' : '/api/salir', desvio ? {ruta: traza.join(',')} : {a: seleccion[0], b: seleccion[1] || -1, ruta: traza.join(',')}); }
  catch (e) { enviando = false; avisar('No hay conexión con el programa.', false); return; }
  enviando = false;
  if (!j.ok) { avisar(j.mensaje, false); dibujar(); return; }
  const nuevo = E;
  E = antes;                                      // Mientras la moto avanza se dibuja el mapa de antes
  animandoViaje = true; ocupado = true;
  dibujarCiudad();
  await animarViaje(nuevo.noche.recorrido, nuevo.noche.entregas, nuevo);
  E = nuevo;
  animandoViaje = false; ocupado = false;
  seleccion = []; traza = [];
  if (E.fase === 'noche' && enDesvio()) {         // Jonatan cerró la calle que seguía
    dibujar();
    avisar(j.mensaje, false);
    ocupado = true;
    await cutin('¡CALLE CERRADA!', 'Jonatan bloqueó tu camino. Traza un desvío.', 1900);
    ocupado = false;
    dibujarNoche();
    return;
  }
  const u = E.noche.ultimoViaje;
  if (u && u.perfecto) brindis(`★ ¡Ruta perfecta! Igualaste al GPS: +${pesos(u.bono)}`, true);
  else if (u && !u.cierre && !antes.noche.gpsVisto && u.costo > u.optimo) brindis(`Tu ruta tardó ${u.costo} ticks; la del GPS, ${u.optimo}.`);
  dibujar();
  avisar(j.mensaje, true);
}

async function escenaUrgente() {
  const o = E.noche.oferta;
  const sobra = o.limite - E.noche.tick - o.ida;
  const v = await abrirEscena(`<h2>¡Pedido urgente!</h2>
    <p style="font-size:15px">${esc(o.nombreCasa)} pide comida ya. Paga <b>${pesos(o.pago)}</b> más ${pesos(o.pagoTick)} por cada tick que sobre, si llega antes del tick <b>${o.limite}</b>.</p>
    <div class="cifras">${cifra('Ida más corta (GPS)', o.ida + ' ticks')}${cifra('Límite', 'tick ' + o.limite)}${cifra('Holgura', sobra + ' ticks', sobra <= 2 ? 'mal' : '')}</div>
    <p class="sub">Si llega frío paga la mitad y resta 3 de reputación. Mientras lo llevas, los otros pedidos se enfrían. Si lo rechazas, no pasa nada.</p>`,
    [{texto: 'No, gracias', clase: 'cr', valor: 0}, {texto: '¡Lo llevo!', clase: 'am', valor: 1}]);
  await accion('/api/urgente', {acepta: v});
  if (v && E.fase === 'noche') { seleccion = [o.id]; traza = [E.noche.pos]; dibujarNoche(); }
}

// =====================================================================
// TIENDA DE MEJORAS Y RESEÑA FINAL
// =====================================================================
const EFECTO = ['Un ayudante lleva a la mesa cada plato listo mientras esté libre.', 'Cada preparación tarda 1 tick menos.', 'Llevas 2 domicilios por viaje.', 'Todos los clientes tienen 4 ticks más de paciencia.'];
function dibujarTienda() {
  $('#tiendaSub').innerHTML = `Tienes <b>${pesos(E.dinero)}</b>. ${E.comproHoy ? 'Ya compraste la mejora de esta jornada.' : 'Elige con cuidado: solo una por jornada.'}`;
  $('#mejoras').innerHTML = E.mejoras.map((m, i) => {
    const puede = !m.comprada && !E.comproHoy && E.dinero >= m.precio;
    return `<div class="mejora ${m.comprada ? 'tiene' : ''}"><svg viewBox="0 0 64 64" aria-hidden="true"><use href="#m${i}"/></svg><b>${esc(m.nombre)}</b><p>${EFECTO[i]}</p>
      ${m.comprada ? '<span class="chip ve">La tienes</span>' : `<button class="btn am chico" data-mejora="${i}" ${puede ? '' : 'disabled'}>Comprar · ${pesos(m.precio)}</button>`}</div>`;
  }).join('');
  document.querySelectorAll('[data-mejora]').forEach(b => b.addEventListener('click', () => accion('/api/mejora', {tipo: b.dataset.mejora})));
  const h = E.historial[E.historial.length - 1];
  $('#resumen').innerHTML = h ? `<div class="cifras">
      ${cifra('Atendidos', h.atendidos, 'bien')}${cifra('Se fueron', h.perdidos, h.perdidos ? 'mal' : '')}${cifra('Domicilios a tiempo', `${h.aTiempo}/${h.pedidos}`)}
      ${cifra('Ventas', pesos(h.ventas))}${cifra('Noche', pesos(h.noche))}${cifra('Reputación', h.reputacion)}</div>
      ${h.sinCobrar ? `<p class="sub">Se perdieron ${pesos(h.sinCobrar)} en cuentas sin cobrar.</p>` : ''}
      <p class="sub">Siguiente: <b>jornada ${E.jornada + 1}</b>. Para ganar necesitas ${pesos(E.metaDinero)} y reputación de ${E.metaRep} al final de la jornada 5.</p>` : '';
}

function tablaRanking(resaltar) {
  if (!E.ranking.length) return '<div class="vacio">Todavía no hay puntajes. ¡Sé el primero!</div>';
  return '<table class="t"><tr><th>#</th><th>Nombre</th><th>Puntos</th><th></th></tr>' +
    E.ranking.map((r, i) => `<tr class="${resaltar === i + 1 ? 'yo' : ''}"><td>${i + 1}</td><td>${esc(r.nombre)}</td><td><b>${r.puntos.toLocaleString('es-CO')}</b></td><td>${r.gano ? '★ Ganó' : ''}</td></tr>`).join('') + '</table>';
}

const SELLO = ['', 'NO RECOMENDADO', 'NO RECOMENDADO', 'REGULAR', 'CASI', 'RECOMENDADO'];
const TITULAR = ['', 'OrderStack: una noche para olvidar', 'OrderStack no aguantó la presión', 'OrderStack sobrevive, pero no convence', 'OrderStack estuvo cerca de la gloria', 'OrderStack conquista al crítico más temido'];
function lineasResena() {               // Tres líneas de la reseña, sacadas de lo que pasó en la partida
  const H = E.historial, suma = k => H.reduce((s, h) => s + h[k], 0);
  const at = suma('atendidos'), pe = suma('perdidos'), ped = suma('pedidos'), aT = suma('aTiempo'), rp = suma('rutasPerfectas');
  const l = [];
  if (!H.length) l.push(`La reputación se derrumbó el primer día: ni el café alcanzó a enfriarse.`);
  else {
    l.push(pe === 0 ? `Atendieron ${at} grupos y nadie se fue sin comer. Impecable.` :
      pe * 4 <= at ? `Atendieron ${at} grupos; solo ${plural(pe, 'se fue', 'se fueron')} sin comer.` : `Atendieron ${at} grupos, pero ${pe} se fueron con hambre y de mal humor.`);
    if (ped) l.push(aT === ped ? `Los ${ped} domicilios llegaron calientes. Todos.` : `${aT} de ${ped} domicilios llegaron calientes${rp ? `, con ${plural(rp, 'ruta perfecta', 'rutas perfectas')}` : ''}.`);
  }
  l.push(E.gano ? `Cerraron con ${pesos(E.dinero)} en caja y la reputación en ${E.reputacion}. Volveré, y pagaré completo.` :
    E.estrellas >= 3 ? `Terminaron con ${pesos(E.dinero)} y reputación de ${E.reputacion}: la meta era ${pesos(E.metaDinero)} y ${E.metaRep}.` :
    `La reputación llegó a 0 en la jornada ${E.jornada}.`);
  return l;
}
function dibujarFinal() {
  const n = E.estrellas || 1;
  $('#titular').textContent = TITULAR[n];
  $('#estrellas').innerHTML = [1, 2, 3, 4, 5].map(i => estrellaSVG(i <= n)).join('');
  $('#resena').innerHTML = lineasResena().map(esc).join('<br>');
  const sello = $('#sello');
  sello.textContent = SELLO[n];
  sello.classList.toggle('verde', n >= 4);
  $('#fraseFinal').textContent = '«' + E.jonatan.frase + '»';
  $('#resultado').textContent = E.gano ? '¡GANASTE!' : n >= 3 ? 'CASI LO LOGRAS' : 'PERDISTE';
  $('#cifras').innerHTML = cifra('Puntaje', E.puntaje.toLocaleString('es-CO')) + cifra('Dinero', pesos(E.dinero), E.dinero >= E.metaDinero ? 'bien' : 'mal') +
    cifra('Reputación', E.reputacion, E.reputacion >= E.metaRep ? 'bien' : 'mal') + cifra('Noches perfectas', E.nochesPerfectas);
  $('#historial').innerHTML = '<tr><th>Jornada</th><th>Atendidos</th><th>Se fueron</th><th>Domicilios a tiempo</th><th>Dinero al cerrar</th></tr>' +
    E.historial.map(h => `<tr><td>${h.jornada}</td><td>${h.atendidos}</td><td>${h.perdidos}</td><td>${h.aTiempo}/${h.pedidos}</td><td>${pesos(h.dinero)}</td></tr>`).join('');
  $('#rankingFinal').innerHTML = tablaRanking(E.puesto) + (E.puesto ? `<p class="sub" style="margin-top:8px">Entraste al ranking en el puesto <b>${E.puesto}</b>.</p>` : '');
}
async function animarResena() {         // Las estrellas caen una por una, luego el sello y la frase de Jonatan
  const est = [...document.querySelectorAll('#estrellas svg')];
  est.forEach(s => s.classList.remove('on'));
  $('#sello').classList.remove('on');
  $('#fraseFinal').classList.remove('on');
  await espera(500);
  for (const s of est) { s.classList.add('on'); await espera(380); }
  await espera(250);
  $('#sello').classList.add('on');
  await espera(450);
  $('#fraseFinal').classList.add('on');
}

// =====================================================================
// PANTALLA DE INICIO, DATOS DEL JUEGO, TECLADO Y ARRANQUE
// =====================================================================
const HISTORIA = ['OrderStack es la cafetería de moda en toda la ciudad...', '...y acaba de llegar El Gran Crítico Jonatan a escribir su reseña.', 'Cinco jornadas para ganarte sus cinco estrellas. ¿Lo logras?'];
let historiaVista = false, historiaReloj = null;
function dibujarInicio(antes) {
  $('#rankingInicio').innerHTML = E.ranking.length ? `<ol>${E.ranking.map(r => `<li>${esc(r.nombre)} <span>${r.puntos.toLocaleString('es-CO')}</span></li>`).join('')}</ol>` : '<div class="vacio">Todavía no hay nadie. ¡Sé el primero en la pizarra!</div>';
  if (antes !== 'inicio') iniciarHistoria();
}
function iniciarHistoria() {
  if (historiaVista) { terminarHistoria(); return; }
  let p = 0, idx = -1;
  clearInterval(historiaReloj);
  $('#bJugar').classList.add('oculto'); $('#carga').classList.remove('oculto'); $('#saltar').classList.remove('oculto');
  historiaReloj = setInterval(() => {
    p += 1.1;
    const nuevo = Math.min(2, Math.floor(p / 34));
    if (nuevo !== idx) { idx = nuevo; const h = $('#historia'); h.textContent = HISTORIA[idx]; h.classList.remove('cambia'); void h.offsetWidth; h.classList.add('cambia'); }
    $('#cargaBarra').style.width = Math.min(100, p) + '%';
    $('#cargaPct').textContent = Math.min(100, Math.floor(p)) + '%';
    if (p >= 100) terminarHistoria();
  }, 60);
}
function terminarHistoria() {
  clearInterval(historiaReloj);
  historiaVista = true;
  $('#historia').textContent = HISTORIA[2];
  $('#carga').classList.add('oculto'); $('#saltar').classList.add('oculto');
  $('#bJugar').classList.remove('oculto');
}
function comidaInicio() {               // Comida volando alrededor de los personajes
  const piezas = [['Pizza', 15, 30, 10, 0], ['Café', 44, 56, 8, .6], ['Papas fritas', 62, 30, 9, 1.1], ['Postre', 9, 58, 7, .3], ['Jugo natural', 55, 15, 7, 1.5], ['Sándwich', 34, 26, 8, .9]];
  $('#comidaInicio').innerHTML = piezas.map(([n, x, y, t, d]) => `<div class="comida" style="left:${x}%;top:${y}%;width:${t}vh;height:${t}vh;animation-delay:-${d}s">${usarComida(n, '')}</div>`).join('') +
    [[30, 12], [70, 8], [26, 40]].map(([x, y], i) => `<svg class="chispa" style="left:${x}%;top:${y}%;animation-delay:${i * .6}s" viewBox="0 0 20 20"><path d="M10 0L12.5 7.5 20 10 12.5 12.5 10 20 7.5 12.5 0 10 7.5 7.5z" fill="#fff" stroke="#2B0A0D" stroke-width="1.5"/></svg>`).join('');
}
function abrirNombre() { $('#mNombre').classList.add('on'); setTimeout(() => $('#nombre').focus(), 50); }
function empezar() {
  $('#mNombre').classList.remove('on');
  reiniciarNochePagina();
  accion('/api/nueva', {nombre: $('#nombre').value});
}
function reiniciarNochePagina() { seleccion = []; traza = []; }

function dibujarDatos() {               // Para el informe: lo que muestran las estructuras de datos
  if (!E || E.fase === 'inicio') { $('#datos').innerHTML = '<div class="vacio">Empieza una partida para ver los datos.</div>'; return; }
  const m = E.memoria, N = E.noche;
  $('#datos').innerHTML = `<table class="t">
    <tr><th>Dato</th><th>Valor</th></tr>
    <tr><td>Casillas guardadas en el mapa (matriz dispersa)</td><td><b>${m.nodos}</b> de ${m.casillas}</td></tr>
    <tr><td>Memoria del mapa disperso / de una matriz normal</td><td>${m.dispersa} / ${m.densa} bytes</td></tr>
    <tr><td>Pedidos en el montículo de la cocina</td><td>${E.pedidos.length}</td></tr>
    <tr><td>Productos en el menú / capacidad del arreglo dinámico</td><td>${E.menu.length} / ${E.capacidadMenu}</td></tr>
    <tr><td>Ventas registradas en la partida</td><td>${E.ventasRegistradas}</td></tr>
    <tr><td>Jornadas en el historial</td><td>${E.historial.length}</td></tr>
    <tr><td>Calles de la ciudad (grafo) / cerradas por Jonatan</td><td>${N.calles.length} / ${N.calles.filter(c => c.cerrada).length}</td></tr>
  </table>
  <h2 style="margin-top:14px">Menú de hoy</h2>
  <table class="t"><tr><th>Producto</th><th>Precio por persona</th><th>Preparación</th></tr>
  ${E.menu.map(p => `<tr><td>${usarComida(p.nombre)} ${esc(p.nombre)}</td><td>${pesos(p.precio)}</td><td>${plural(p.prep, 'tick', 'ticks')}</td></tr>`).join('')}</table>`;
}

// ---------- Teclado ----------
document.addEventListener('keydown', ev => {
  if (ev.target.tagName === 'INPUT') { if (ev.key === 'Enter') empezar(); return; }
  if ($('#escena').classList.contains('on')) {
    if (ev.key === 'Enter' || ev.key === ' ') { ev.preventDefault(); const b = $('#escenaCard .acciones .btn:last-child'); if (b) b.click(); }
    return;
  }
  if (ev.key === 'Escape') { document.querySelectorAll('.modal').forEach(m => m.classList.remove('on')); return; }
  if (ev.key === 'h' || ev.key === 'H') { $('#mAyuda').classList.toggle('on'); return; }
  if (!E || document.querySelector('.modal.on')) return;
  if (E.fase === 'inicio') {
    if (ev.key === ' ' || ev.key === 'Enter') { ev.preventDefault(); if (!historiaVista) terminarHistoria(); else abrirNombre(); }
    return;
  }
  if (ocupado) return;
  if (E.fase === 'dia') {
    if (performance.now() < pausaTeclas) { ev.preventDefault(); return; }
    const dirs = {ArrowUp: 'A', ArrowDown: 'B', ArrowLeft: 'I', ArrowRight: 'D', w: 'A', s: 'B', a: 'I', d: 'D', W: 'A', S: 'B', A: 'I', D: 'D'};
    if (dirs[ev.key]) {
      ev.preventDefault();
      if (dirs[ev.key] === 'I') dirJugador = -1;
      if (dirs[ev.key] === 'D') dirJugador = 1;
      ultimoMovimiento = performance.now();
      accion('/api/mover', {dir: dirs[ev.key]});
    }
    else if (ev.key === 'e' || ev.key === 'E') { ev.preventDefault(); accion('/api/usar'); }
    else if (ev.key === ' ') { ev.preventDefault(); accion('/api/esperar'); }
    else if (ev.key >= '1' && ev.key <= '6') { ev.preventDefault(); accion('/api/sentar', {mesa: ev.key}); }
  } else if (E.fase === 'noche') {
    if (ev.key === 'Backspace' && traza.length > 1) { ev.preventDefault(); traza.pop(); dibujarNoche(); }
    else if (ev.key === 'Enter') { ev.preventDefault(); salirMoto(); }
  }
});

// ---------- Botones ----------
$('#escenaInicio').addEventListener('click', ev => { if (!historiaVista && ev.target.id !== 'bJugar') terminarHistoria(); });
$('#bJugar').addEventListener('click', abrirNombre);
$('#bEmpezar').addEventListener('click', empezar);
$('#bCerrar').addEventListener('click', () => { if (!ocupado) accion('/api/cerrar'); });
$('#bSalir').addEventListener('click', salirMoto);
$('#bDeshacer').addEventListener('click', () => { if (traza.length > 1 && !ocupado) { traza.pop(); dibujarNoche(); } });
$('#bBorrar').addEventListener('click', () => { if (!ocupado) { traza = [E.noche.pos]; dibujarNoche(); } });
$('#bRegreso').addEventListener('click', () => { if (!ocupado) completarRegreso(); });
$('#bGps').addEventListener('click', verGps);
$('#bSiguiente').addEventListener('click', () => accion('/api/siguiente'));
$('#bOtra').addEventListener('click', () => { historiaVista = true; accion('/api/inicio'); });
$('#bAyuda').addEventListener('click', () => $('#mAyuda').classList.add('on'));
$('#bDatos').addEventListener('click', () => { dibujarDatos(); $('#mDatos').classList.add('on'); });
document.querySelectorAll('[data-cerrar]').forEach(b => b.addEventListener('click', () => b.closest('.modal').classList.remove('on')));
document.querySelectorAll('.modal').forEach(m => m.addEventListener('click', ev => { if (ev.target === m) m.classList.remove('on'); }));
window.addEventListener('resize', () => { if (!E) return; if (E.fase === 'dia') dibujarDia(); else if (E.fase === 'noche' && !animandoViaje) dibujarNoche(); });

// ---------- Arranque ----------
prepararSimbolos();
prepararImagenes();
dibujarRayos(); dibujarCafeFondo(); dibujarMeseroInicio(); dibujarClientesInicio(); dibujarSilueta(); comidaInicio();
requestAnimationFrame(animar);
fetch('/api/estado').then(r => r.json()).then(e => { E = e; dibujar(); });
</script>
</body>
</html>
)HTML";

#endif                    // Fin de la protección contra lecturas repetidas
