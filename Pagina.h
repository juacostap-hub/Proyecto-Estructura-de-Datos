#ifndef PAGINA_H          // Evita que este archivo se lea más de una vez al compilar
#define PAGINA_H          // Marca que este archivo ya fue leído

// PÁGINA DE LA INTERFAZ GRÁFICA
// Es la página completa (HTML + CSS + JavaScript) guardada como un texto de C++.
// El servidor (Servidor.cpp) se la entrega al navegador. Para cambiar colores, textos o el diseño,
// se edita aquí: los colores están al inicio, en el bloque ":root" del CSS (paleta de OrderStack: naranja, rojo y crema).

static const char* PAGINA_HTML = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>OrderStack · Simulador de Cafetería</title>
<style>
:root{
  /* Colores de OrderStack (tomados de la portada) */
  --naranja:#FCB14E; --rojo:#DC0E20; --rojo-osc:#A30A18; --crema:#F1EAD1; --papel:#FBF6E4;
  --tinta:#3B0F12; --tinta2:#7B2A2E; --tinta3:#A98474; --linea:#DDCFA5;
  --verde:#5C8A2E; --verdeBg:#E2EDCB; --verdeTx:#3F6B1F; --rojoBg:#FBD5D6; --suave:#FFE5B4;
  --sombra:5px 6px 0 rgba(163,10,24,.28);
  --titulo:Impact,'Haettenschweiler','Arial Narrow Bold','Arial Black',sans-serif;
}
*{box-sizing:border-box}
html,body{margin:0}
body{background:var(--naranja);color:var(--tinta);font:15px/1.45 'Segoe UI',system-ui,-apple-system,Roboto,Arial,sans-serif;min-height:100vh;display:flex;flex-direction:column}
button{font:inherit;cursor:pointer}
header{background:var(--rojo);color:var(--papel);position:sticky;top:0;z-index:20;box-shadow:0 4px 14px rgba(59,15,18,.35)}
.hd{max-width:1480px;margin:0 auto;display:flex;align-items:center;gap:20px;padding:12px 28px;flex-wrap:wrap}
.cuadros{height:28px;background:repeating-conic-gradient(var(--rojo) 0 25%,var(--naranja) 0 50%) 0 0/28px 28px}
.pie{margin-top:auto}
.logo{display:flex;align-items:center;gap:12px}
.logo .ico{width:48px;height:48px;border-radius:50%;background:var(--crema);display:grid;place-items:center;font-size:25px;border:3px solid var(--rojo-osc)}
.logo b{font-family:var(--titulo);font-weight:400;font-size:30px;letter-spacing:1.5px;display:block;line-height:1}
.logo small{color:#FFD9A0;font-size:12.5px}
nav{display:flex;gap:6px;flex-wrap:wrap}
.tab{border:0;background:transparent;color:#FFE9C4;padding:10px 16px;border-radius:999px;font-weight:700;display:flex;gap:8px;align-items:center;transition:.15s}
.tab:hover{background:rgba(255,255,255,.18)}
.tab.on{background:var(--crema);color:var(--rojo);box-shadow:0 3px 0 var(--rojo-osc)}
.der{margin-left:auto;display:flex;gap:12px;align-items:center}
.kpi{background:var(--crema);border-radius:12px;padding:6px 14px;font-size:12px;color:var(--tinta2);line-height:1.2}
.kpi b{display:block;font-family:var(--titulo);font-weight:400;letter-spacing:.5px;font-size:21px;color:var(--rojo)}
.reset{border:2px solid var(--crema);background:transparent;color:var(--crema);border-radius:10px;padding:8px 12px;font-weight:700}
.reset:hover{background:rgba(255,255,255,.18)}
main{max-width:1480px;width:100%;margin:0 auto;padding:24px 28px 40px;flex:1}
.salon{display:grid;grid-template-columns:340px 1fr 380px;gap:20px;align-items:start}
.dom{display:grid;grid-template-columns:1fr 1fr 360px;gap:20px;align-items:start}
.dos{display:grid;grid-template-columns:1fr 1fr;gap:20px;align-items:start}
@media(max-width:1180px){.salon,.dom{grid-template-columns:1fr 1fr}.salon>:last-child,.dom>:last-child{grid-column:1/-1}}
@media(max-width:760px){.salon,.dom,.dos{grid-template-columns:1fr}main{padding:16px}}
.card{background:var(--papel);border:2.5px solid var(--rojo);border-radius:18px;padding:18px;box-shadow:var(--sombra)}
.card+.card{margin-top:20px}
.dos .card+.card,.dom .card+.card{margin-top:0}
.card h2{margin:0;font-family:var(--titulo);font-weight:400;font-size:21px;letter-spacing:.8px;text-transform:uppercase;color:var(--rojo);display:flex;align-items:center;gap:8px}
.sub{color:var(--tinta2);font-size:13px;margin:3px 0 12px}
.btn{border:0;border-radius:12px;padding:11px 16px;font-weight:800;display:inline-flex;align-items:center;justify-content:center;gap:8px;transition:transform .08s,background .15s}
.btn:active{transform:translateY(2px)}
.btn.p{background:var(--naranja);color:var(--tinta);box-shadow:0 4px 0 #D98A24}
.btn.p:hover{background:#FFBE63}
.btn.v{background:var(--rojo);color:var(--papel);box-shadow:0 4px 0 var(--rojo-osc)}
.btn.v:hover{background:#EC1A2C}
.btn.r{background:var(--rojo-osc);color:var(--papel);box-shadow:0 4px 0 #6E0610}
.btn.s{background:var(--crema);color:var(--rojo-osc)}
.btn.s:hover{background:#E6DCB8}
.btn.g{width:100%}
.btn.m{padding:7px 12px;font-size:13px;border-radius:10px}
.btn[disabled]{opacity:.45;cursor:not-allowed;box-shadow:none}
.fila{display:flex;gap:10px}
.fila>*{flex:1}
.pill{font-size:11.5px;font-weight:800;letter-spacing:.4px;padding:3px 10px;border-radius:999px;white-space:nowrap}
.pill.verde{background:var(--verdeBg);color:var(--verdeTx)}
.pill.rojo{background:var(--rojoBg);color:var(--rojo-osc)}
.pill.oro{background:var(--suave);color:#8A4B00}
.sec{display:flex;align-items:center;justify-content:space-between;flex-wrap:wrap;gap:2px 10px;margin:16px 0 8px;font-weight:800;font-size:14px}
.sec small{font-weight:600;color:var(--tinta2)}
.vacio{color:var(--tinta3);font-style:italic;padding:8px 2px;font-size:14px}
.cli{display:flex;align-items:center;gap:10px;padding:9px 10px;border-radius:12px;background:var(--crema);margin-bottom:8px;border:2px solid transparent}
.cli.sigue{border-color:var(--rojo);background:var(--suave)}
.av{width:36px;height:36px;border-radius:50%;display:grid;place-items:center;color:#fff;font-weight:800;flex:none}
.cli b{display:block;line-height:1.15}
.cli small{color:var(--tinta2)}
.tag{margin-left:auto;font-size:11px;font-weight:800;border-radius:999px;padding:3px 9px;white-space:nowrap}
.tag.sigue{background:var(--rojo);color:var(--papel)}
.t3{background:var(--rojo);color:var(--papel)}
.t2{background:var(--naranja);color:var(--tinta)}
.t1{background:#E3D8B5;color:var(--rojo-osc)}
.regla{background:var(--crema);border-radius:12px;padding:10px 12px;font-size:13px;color:var(--tinta2);margin-top:12px;display:flex;align-items:center;gap:10px}
.regla>span:first-child{white-space:nowrap;flex:none}
.col{display:grid;gap:20px;align-content:start}
.col .card+.card{margin-top:0}
.dot{width:14px;height:14px;border-radius:50%;border:2.5px solid var(--rojo);display:inline-block;margin-right:4px}
.dot.on{background:var(--rojo)}
.mesas{display:grid;grid-template-columns:repeat(auto-fill,minmax(190px,1fr));gap:14px;margin-top:6px}
.mesa{border:2.5px solid var(--linea);border-radius:16px;padding:14px;background:#FFFDF4;transition:.15s;display:block;text-align:left;width:100%;color:var(--tinta)}
.mesa.libre{border-color:#A9C97A;background:linear-gradient(#FFFDF4,#F4F8E8)}
.mesa.ocupada{border-color:var(--rojo);background:linear-gradient(#FFFDF4,#FDE9E4);cursor:pointer}
.mesa.ocupada:hover{transform:translateY(-3px);box-shadow:var(--sombra)}
.mesa.sel{outline:4px solid var(--naranja);outline-offset:2px}
.mtop{display:flex;align-items:center;gap:10px;margin-bottom:10px}
.mnum{width:46px;height:46px;border-radius:14px;display:grid;place-items:center;font-family:var(--titulo);font-weight:400;font-size:25px;color:#fff}
.libre .mnum{background:var(--verde)} .ocupada .mnum{background:var(--rojo)}
.mesa small{display:block;color:var(--tinta2)}
.mesa .l2{font-size:13.5px;margin-top:3px}
.platos{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.plato{border:2px solid var(--linea);background:#FFFDF4;border-radius:14px;padding:12px;text-align:left;transition:.12s;color:var(--tinta)}
.plato:hover{border-color:var(--rojo);transform:translateY(-2px);box-shadow:var(--sombra)}
.plato .em{font-size:28px;display:block}
.plato b{display:block;font-size:13.5px;line-height:1.2;margin:4px 0 2px}
.plato span.pr{color:var(--rojo);font-weight:800}
.pila .it{display:flex;justify-content:space-between;align-items:center;gap:8px;padding:9px 12px;border-radius:10px;background:var(--crema);margin-bottom:6px}
.pila .it.tope{background:var(--suave);border:2px dashed var(--rojo)}
.pila .it span.tp{font-size:10.5px;font-weight:800;background:var(--rojo);color:var(--papel);border-radius:999px;padding:2px 8px;margin-right:6px}
.tot{display:flex;justify-content:space-between;align-items:baseline;font-weight:800;font-size:18px;padding:12px 4px 4px;border-top:2px dashed var(--linea);margin-top:10px}
.tot span:last-child{font-family:var(--titulo);font-weight:400;font-size:26px;color:var(--rojo);letter-spacing:.5px}
.in{width:100%;padding:11px 12px;border:2px solid var(--linea);border-radius:12px;font:inherit;background:#FFFDF4;color:var(--tinta)}
.in:focus{outline:none;border-color:var(--rojo);box-shadow:0 0 0 3px rgba(220,14,32,.2)}
.lb{font-weight:700;font-size:13px;margin:12px 0 6px;display:block}
.chips{display:grid;grid-template-columns:1fr 1fr;gap:8px}
.chip{border:2px solid var(--linea);border-radius:12px;padding:10px;background:#FFFDF4;text-align:center;font-weight:700;font-size:13px;color:var(--tinta)}
.chip.on{border-color:var(--rojo);background:var(--suave)}
.step{display:flex;align-items:center;gap:10px}
.step button{width:42px;height:42px;border-radius:12px;border:2px solid var(--linea);background:#FFFDF4;font-size:20px;font-weight:800;color:var(--rojo)}
.step b{min-width:34px;text-align:center;font-size:20px}
.fondo{position:fixed;inset:0;background:rgba(59,15,18,.62);display:none;place-items:center;z-index:50}
.fondo.on{display:grid}
.modal{background:var(--papel);border:3px solid var(--rojo);border-radius:20px;padding:24px;width:min(460px,92vw);box-shadow:8px 9px 0 rgba(163,10,24,.45);max-height:90vh;overflow:auto}
.modal h3{margin:0 0 4px;font-family:var(--titulo);font-weight:400;font-size:26px;letter-spacing:.8px;text-transform:uppercase;color:var(--rojo)}
.ticket{font-family:'Courier New',monospace;background:#FFFDF4;border:2px dashed var(--tinta3);padding:16px;border-radius:12px;margin:12px 0}
.ticket .ln{display:flex;justify-content:space-between;padding:3px 0;border-bottom:1px dotted #D9C79A}
.ticket .tt{display:flex;justify-content:space-between;font-weight:800;font-size:18px;margin-top:10px;color:var(--rojo)}
#toast{position:fixed;left:50%;bottom:26px;transform:translate(-50%,30px);opacity:0;pointer-events:none;background:var(--tinta);color:#fff;padding:12px 20px;border-radius:14px;box-shadow:0 10px 30px rgba(0,0,0,.35);transition:.25s;z-index:60;max-width:90vw;font-weight:700;border:2px solid var(--crema)}
#toast.on{opacity:1;transform:translate(-50%,0)}
#toast.bien{background:var(--verdeTx)}
#toast.mal{background:var(--rojo-osc)}
.emp{display:flex;align-items:center;gap:10px;padding:9px 10px;border-radius:12px;background:var(--crema);margin-bottom:8px}
.emp b{display:block;line-height:1.15}
.emp small{color:var(--tinta2)}
.emp .acc{margin-left:auto;display:flex;gap:8px;align-items:center}
table{width:100%;border-collapse:collapse;font-size:14px}
th{text-align:left;color:var(--tinta2);font-size:12px;text-transform:uppercase;letter-spacing:.5px;padding:6px 8px}
td{padding:9px 8px;border-top:1px solid var(--linea)}
tr.hallado td{background:var(--suave)}
.stats{display:grid;grid-template-columns:repeat(3,1fr);gap:20px}
@media(max-width:900px){.stats{grid-template-columns:1fr}}
.stat{border-radius:20px;padding:26px;box-shadow:var(--sombra);border:2.5px solid var(--rojo-osc);font-weight:700}
.stat .n{font-family:var(--titulo);font-weight:400;font-size:54px;letter-spacing:1px;line-height:1.1;margin-top:6px}
.stat .e{font-size:34px}
.stat.a{background:var(--rojo);color:var(--papel)}
.stat.b{background:var(--crema);color:var(--rojo)}
.stat.c{background:#5A1218;color:var(--crema)}
.slots{display:flex;gap:8px;flex-wrap:wrap;margin:34px 0 30px}
.slot{width:112px;border:2px dashed var(--linea);border-radius:12px;padding:10px 6px;text-align:center;position:relative;min-height:72px;background:#FFFDF4}
.slot.on{border:2px solid var(--tinta2);background:#fff}
.slot.ocu{background:var(--rojoBg)}
.slot .ix{position:absolute;top:-10px;left:8px;background:var(--papel);font-size:11px;font-weight:700;color:var(--tinta3);padding:0 5px}
.slot b{display:block;font-size:14px}
.slot small{color:var(--tinta2);font-size:12px}
.mk{position:absolute;left:0;right:0;font-size:11px;font-weight:800;color:var(--rojo)}
.mk.top{top:-26px}
.mk.bot{bottom:-26px}
.cadena{display:flex;align-items:center;gap:6px;flex-wrap:wrap;margin:14px 0 6px}
.nodo{border:2px solid var(--tinta2);border-radius:10px;padding:8px 12px;background:#fff;font-weight:700;font-size:14px;position:relative}
.nodo small{display:block;font-weight:600;color:var(--tinta2);font-size:11.5px}
.flecha{color:var(--rojo);font-weight:800}
.nulo{color:var(--tinta3);font-style:italic}
.lblc{font-size:11px;font-weight:800;color:var(--rojo);margin-top:4px}
.nivel{display:flex;align-items:center;gap:14px;border-top:1px dashed var(--linea);padding:4px 0}
.nivel:first-of-type{border-top:0}
.lblnv{width:150px;font-size:13px;color:var(--tinta2);flex:none}
.apila{display:flex;flex-direction:column;gap:6px;max-width:260px;margin-top:8px}
.nota{background:var(--crema);border-radius:12px;padding:10px 12px;font-size:13px;color:var(--tinta2);margin-top:10px}
code{background:var(--crema);border-radius:6px;padding:1px 6px;font-size:12.5px}
</style>
</head>
<body>
<header>
  <div class="hd">
    <div class="logo"><div class="ico">🍴</div><div><b>ORDERSTACK</b><small>Simulador de cafetería · Estructuras de Datos</small></div></div>
    <nav id="nav">
      <button class="tab" data-t="salon" onclick="go('salon')">🍽️ Salón</button>
      <button class="tab" data-t="domicilios" onclick="go('domicilios')">🛵 Domicilios</button>
      <button class="tab" data-t="admin" onclick="go('admin')">⚙️ Administración</button>
      <button class="tab" data-t="ventas" onclick="go('ventas')">💰 Ventas</button>
      <button class="tab" data-t="estructuras" onclick="go('estructuras')">🧩 Estructuras</button>
    </nav>
    <div class="der">
      <div class="kpi">Vendido hoy<b id="kv">$0</b></div>
      <button class="reset" onclick="reiniciar()">↺ Reiniciar</button>
    </div>
  </div>
  <div class="cuadros"></div>
</header>
<main id="vista"><div class="vacio">Conectando con el programa en C++…</div></main>
<div class="cuadros pie"></div>
<div id="toast"></div>
<div class="fondo" id="fondo" onclick="if(event.target===this)cerrar()"><div class="modal" id="modal"></div></div>

<script>
const $ = s => document.querySelector(s);
let E = null;
let ui = {tab:'salon', mesa:null, cli:'', dir:'', hallado:-1, tarjeta:0, personas:2};

const money = n => '$' + Number(n).toLocaleString('es-CO');
const h = s => String(s).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const COLORES = ['#DC0E20','#E07A12','#7B2A2E','#B8560F','#5C8A2E','#5A3A2E'];
const color = n => COLORES[[...String(n)].reduce((a,c)=>a+c.charCodeAt(0),0) % COLORES.length];
const av = n => `<div class="av" style="background:${color(n)}">${h(String(n).trim().charAt(0).toUpperCase())}</div>`;
function emoji(n){
  n = n.toLowerCase();
  const m = [['hamburg','🍔'],['alita','🍗'],['limon','🍋'],['helado','🍨'],['pizza','🍕'],['caf','☕'],['torta','🍰'],['pastel','🥐'],['jugo','🧃'],['agua','💧'],['ensalada','🥗'],['papa','🍟'],['perro','🌭'],['sandwich','🥪'],['sándwich','🥪'],['pasta','🍝'],['taco','🌮'],['gaseosa','🥤'],['cerveza','🍺'],['galleta','🍪']];
  for(const [k,e] of m) if(n.includes(k)) return e;
  return '🍽️';
}

// ---------- comunicación con el programa en C++ ----------
async function api(ruta, datos, metodo){
  metodo = metodo || 'POST';
  try{
    const op = {method: metodo};
    if(metodo === 'POST'){ op.headers = {'Content-Type':'application/x-www-form-urlencoded'}; op.body = new URLSearchParams(datos || {}); }
    const r = await fetch(ruta, op);
    const j = await r.json();
    const est = j.estado || (j.menu ? j : null);
    if(est){ E = est; render(); }
    if(j.mensaje) toast(j.mensaje, j.ok);
    if(j.cuenta) mostrarCuenta(j.cuenta);
    return j;
  }catch(e){
    toast('No se pudo hablar con el programa en C++. ¿Sigue abierto?', false);
  }
}
let tt;
function toast(m, ok){
  const t = $('#toast');
  t.textContent = m;
  t.className = 'on ' + (ok ? 'bien' : 'mal');
  clearTimeout(tt);
  tt = setTimeout(() => t.className = '', 3400);
}
function go(t){ ui.tab = t; location.hash = t; render(); window.scrollTo(0,0); }
function abrir(html){ $('#modal').innerHTML = html; $('#fondo').classList.add('on'); }
function cerrar(){ $('#fondo').classList.remove('on'); }
function reiniciar(){ if(confirm('¿Reiniciar la cafetería con los datos iniciales?')){ ui.mesa = null; ui.cli = ''; ui.dir = ''; api('/api/reiniciar'); } }

// ---------- datos derivados ----------
function quienSigue(){
  if(E.vipSeguidos >= E.vipPorComun && E.filaComun.length) return E.filaComun[0];
  if(E.filaVip.length) return E.filaVip[0];
  if(E.filaComun.length) return E.filaComun[0];
  return null;
}
function mesaSel(){ return E.mesas.find(m => m.id === ui.mesa && m.ocupada) || null; }
function primerLibre(g){ const o = g.orden.find(e => !e.ocupado); return o ? o.id : -1; }

// ---------- piezas de pantalla ----------
function itemCliente(c, sigue){
  const t = c.tarjeta ? `<span class="tag t${c.tarjeta}">${h(c.tarjetaNombre)}</span>` : '';
  const s = sigue ? '<span class="tag sigue">Sigue</span>' : '';
  return `<div class="cli ${sigue ? 'sigue' : ''}">${av(c.nombre)}<div><b>${h(c.nombre)}</b><small>👥 ${c.personas} · Turno ${c.turno}</small></div>${s || t}</div>`;
}
function platos(fn){
  if(!E.menu.length) return '<div class="vacio">El menú está vacío. Agrega platos en Administración.</div>';
  return `<div class="platos">${E.menu.map(p => `<button class="plato" onclick="${fn}(${p.i})"><span class="em">${emoji(p.nombre)}</span><b>${h(p.nombre)}</b><span class="pr">${money(p.precio)}</span></button>`).join('')}</div>`;
}
function pila(items, total){
  if(!items.length) return '<div class="vacio">Aún no hay productos. Toca un plato para agregarlo.</div>';
  const filas = items.map((p, i) => ({p, i})).reverse().map(({p, i}) =>
    `<div class="it ${i === items.length - 1 ? 'tope' : ''}"><span>${i === items.length - 1 ? '<span class="tp">TOPE</span>' : ''}${emoji(p.nombre)} ${h(p.nombre)}</span><b>${money(p.precio)}</b></div>`).join('');
  return `<div class="pila">${filas}</div><div class="tot"><span>Total</span><span>${money(total)}</span></div>`;
}
function empleado(tipo, e, sigue){
  const est = e.ocupado ? '<span class="pill rojo">OCUPADO</span>' : '<span class="pill verde">LIBRE</span>';
  const lib = e.ocupado ? `<button class="btn s m" onclick="liberar('${tipo}',${e.id})">Liberar</button>` : '';
  const sg = sigue ? '<span class="tag sigue">Sigue</span>' : '';
  return `<div class="emp">${av(e.nombre)}<div><b>${h(e.nombre)}</b><small>ID ${e.id} · ${e.servicios} servicio(s)</small></div><div class="acc">${sg}${est}${lib}</div></div>`;
}

// ---------- vistas ----------
function vSalon(){
  const sel = mesaSel();
  const quien = quienSigue();
  const vip = E.filaVip.map((c, i) => itemCliente(c, quien && quien.turno === c.turno)).join('') || '<div class="vacio">Nadie en la fila VIP</div>';
  const com = E.filaComun.map((c, i) => itemCliente(c, quien && quien.turno === c.turno)).join('') || '<div class="vacio">Nadie en la fila común</div>';
  const dots = Array.from({length: E.vipPorComun}, (_, i) => `<span class="dot ${i < E.vipSeguidos ? 'on' : ''}"></span>`).join('');
  const mesas = E.mesas.map(m => {
    const libre = !m.ocupada;
    const l2 = libre ? 'Disponible' : `🧑‍🍳 ${h(m.mesero || '—')}`;
    const l3 = libre ? '' : `<div class="l2">🧾 ${m.comanda.length} producto(s) · <b>${money(m.total)}</b></div>`;
    return `<button class="mesa ${libre ? 'libre' : 'ocupada'} ${sel && sel.id === m.id ? 'sel' : ''}" onclick="selMesa(${m.id})"><div class="mtop"><div class="mnum">${m.id}</div><div><span class="pill ${libre ? 'verde' : 'rojo'}">${libre ? 'LIBRE' : 'OCUPADA'}</span><small>👥 ${m.capacidad} personas</small></div></div><div class="l2">${l2}</div>${l3}</button>`;
  }).join('');
  const turnos = E.meseros.orden.map(e => `<span class="pill ${e.ocupado ? 'rojo' : 'verde'}" style="margin:0 6px 6px 0;display:inline-block">${h(e.nombre)}${e.id === primerLibre(E.meseros) ? ' · sigue' : ''}</span>`).join('');
  let der;
  if(sel){
    der = `<div class="card"><h2>🧾 Comanda · Mesa ${sel.id}</h2><div class="sub">Atiende ${h(sel.mesero || '—')} · toca un plato para apilarlo</div>
      ${platos('agregarComanda')}
      <div class="sec">Pila de la comanda <small>último en entrar, primero en salir</small></div>
      ${pila(sel.comanda, sel.total)}
      <div class="fila" style="margin-top:14px"><button class="btn s" onclick="deshacer()" ${sel.comanda.length ? '' : 'disabled'}>↶ Deshacer último</button><button class="btn v" onclick="pagar()">💳 Pagar cuenta</button></div></div>`;
  } else {
    der = `<div class="card"><h2>🧾 Comanda</h2><div class="sub">Aquí se toma el pedido de una mesa</div><div class="vacio" style="padding:30px 6px;text-align:center">${E.mesas.some(m => m.ocupada) ? 'Selecciona una mesa ocupada para tomar su comanda.' : 'Llama a un cliente para que ocupe una mesa.'}</div></div>`;
  }
  return `<div class="salon">
    <div class="card"><h2>👥 Fila de espera</h2><div class="sub">Dos filas: VIP (cola con prioridad) y común (cola normal)</div>
      <button class="btn p g" onclick="abrirRegistro()">➕ Registrar cliente</button>
      <button class="btn v g" style="margin-top:12px" onclick="llamar()" ${quien ? '' : 'disabled'}>📣 ${quien ? 'Llamar a ' + h(quien.nombre) : 'Llamar al siguiente'}</button>
      <div class="regla"><span>${dots}</span><span>Después de ${E.vipPorComun} VIP seguidos le toca a un cliente común</span></div>
      <div class="sec">⭐ Fila VIP <small>${E.filaVip.length} esperando</small></div>${vip}
      <div class="sec">Fila común <small>${E.filaComun.length} esperando</small></div>${com}
    </div>
    <div><div class="card"><h2>🪑 Mesas</h2><div class="sub">Toca una mesa ocupada para ver su comanda</div><div class="mesas">${mesas}</div></div>
      <div class="card"><h2>🧑‍🍳 Turno de los meseros</h2><div class="sub">Cola circular: el primero libre atiende y pasa al final</div>${turnos}</div></div>
    ${der}</div>`;
}
function vDomicilios(){
  const d = E.domicilio;
  const sig = primerLibre(E.domiciliarios);
  const lista = E.domiciliarios.orden.map(e => empleado('domiciliario', e, e.id === sig)).join('') || '<div class="vacio">No hay domiciliarios.</div>';
  return `<div class="dom">
    <div class="card"><h2>🍴 Menú</h2><div class="sub">Toca un plato para agregarlo al pedido</div>${platos('domAgregar')}</div>
    <div class="card"><h2>🛵 Pedido a domicilio</h2><div class="sub">Se arma con una pila: el último producto se puede deshacer</div>
      ${pila(d.productos, d.total)}
      <button class="btn s g" style="margin-top:12px" onclick="domDeshacer()" ${d.productos.length ? '' : 'disabled'}>↶ Deshacer último</button>
      <label class="lb">Nombre del cliente</label><input class="in" id="d-cli" value="${h(ui.cli)}" oninput="ui.cli=this.value" placeholder="Ej: Juan Pérez">
      <label class="lb">Dirección de entrega</label><input class="in" id="d-dir" value="${h(ui.dir)}" oninput="ui.dir=this.value" placeholder="Ej: Calle 10 #5-20">
      <button class="btn v g" style="margin-top:16px" onclick="domConfirmar()">✅ Confirmar y asignar domiciliario</button></div>
    <div class="card"><h2>🏍️ Domiciliarios</h2><div class="sub">Cola circular · ${E.domiciliarios.libres} libre(s) de ${E.domiciliarios.cantidad}</div>${lista}</div>
  </div>`;
}
function vAdmin(){
  const sm = primerLibre(E.meseros), sd = primerLibre(E.domiciliarios);
  const grupo = (tipo, g, titulo, emo, sig) => `<div class="card"><h2>${emo} ${titulo}</h2><div class="sub">${g.cantidad} de ${g.capacidad} · el primero de la lista es el siguiente en turno</div>
    ${g.orden.map(e => empleado(tipo, e, e.id === sig)).join('') || '<div class="vacio">Sin empleados</div>'}
    <div class="fila" style="margin-top:12px"><input class="in" id="np-${tipo}" placeholder="Nombre del nuevo ${tipo}" onkeydown="if(event.key==='Enter')agregarPersonal('${tipo}')"><button class="btn p" style="flex:none" onclick="agregarPersonal('${tipo}')">➕ Agregar</button></div></div>`;
  const filas = E.menu.map(p => `<tr class="${ui.hallado === p.i ? 'hallado' : ''}"><td>[${p.i}]</td><td>${emoji(p.nombre)} ${h(p.nombre)}</td><td><b>${money(p.precio)}</b></td></tr>`).join('');
  const mesas = E.mesas.map(m => `<span class="pill ${m.ocupada ? 'rojo' : 'verde'}" style="display:inline-block;margin:0 8px 8px 0;font-size:13px">Mesa ${m.id} · 👥 ${m.capacidad}</span>`).join('');
  return `<div class="dos">
    <div>${grupo('mesero', E.meseros, 'Meseros', '🧑‍🍳', sm)}<div style="height:20px"></div>${grupo('domiciliario', E.domiciliarios, 'Domiciliarios', '🏍️', sd)}</div>
    <div><div class="card"><h2>📋 Menú de productos</h2><div class="sub">Arreglo dinámico: ${E.menu.length} plato(s) · caben ${E.menuTamano}</div>
      <table><tr><th>Pos.</th><th>Plato</th><th>Precio</th></tr>${filas || '<tr><td colspan="3" class="vacio">El menú está vacío</td></tr>'}</table>
      <label class="lb">Agregar plato</label>
      <div class="fila"><input class="in" id="pl-nombre" placeholder="Nombre" style="flex:2"><input class="in" id="pl-precio" type="number" min="0" placeholder="Precio"></div>
      <div class="fila" style="margin-top:10px"><button class="btn p" onclick="agregarPlato()">➕ Agregar plato</button><button class="btn s" onclick="quitarPlato()">🗑️ Quitar el último</button></div>
      <label class="lb">Buscar plato por nombre</label>
      <div class="fila"><input class="in" id="pl-buscar" placeholder="Nombre exacto" style="flex:2" onkeydown="if(event.key==='Enter')buscarPlato()"><button class="btn s" onclick="buscarPlato()">🔎 Buscar</button></div></div>
      <div class="card" style="margin-top:20px"><h2>🪑 Mesas</h2><div class="sub">Arreglo dinámico: ${E.mesas.length} mesa(s) · caben ${E.mesasTamano}</div>${mesas}
      <div class="fila" style="margin-top:6px"><input class="in" id="mesa-cap" type="number" min="1" placeholder="Capacidad (personas)" onkeydown="if(event.key==='Enter')agregarMesa()"><button class="btn p" style="flex:none" onclick="agregarMesa()">➕ Agregar mesa</button></div></div></div>
  </div>`;
}
function vVentas(){
  const v = E.ventas;
  return `<div class="stats">
    <div class="stat a"><div class="e">💰</div>Total vendido<div class="n">${money(v.total)}</div></div>
    <div class="stat b"><div class="e">🧾</div>Cuentas del salón<div class="n">${v.cuentas}</div></div>
    <div class="stat c"><div class="e">🛵</div>Pedidos a domicilio<div class="n">${v.domicilios}</div></div>
  </div><div class="nota" style="margin-top:20px">Las ventas se suman cuando se paga una cuenta del salón o se confirma un pedido a domicilio.</div>`;
}
function ranuras(g){
  return `<div class="slots">${g.ranuras.map((r, k) => {
    const fr = k === g.frente, fi = k === g.final;
    return `<div class="slot ${r.usada ? 'on' : ''} ${r.usada && r.ocupado ? 'ocu' : ''}"><span class="ix">${k}</span>${fr ? '<div class="mk top">FRENTE ▼</div>' : ''}${r.usada ? `<b>${h(r.nombre)}</b><small>${r.ocupado ? 'ocupado' : 'libre'} · ${r.servicios}</small>` : '<small>vacía</small>'}${fi ? '<div class="mk bot">▲ FINAL</div>' : ''}</div>`;
  }).join('')}</div>`;
}
function cadena(lista, titulo){
  if(!lista.length) return '<div class="cadena"><span class="nulo">vacía</span></div>';
  return `<div class="cadena">${lista.map((c, i) => `<div class="nodo">${h(c.nombre)}<small>${c.tarjeta ? h(c.tarjetaNombre) : 'sin tarjeta'}</small>${i === 0 ? '<div class="lblc">cabeza</div>' : ''}${i === lista.length - 1 ? '<div class="lblc">último</div>' : ''}</div><span class="flecha">→</span>`).join('')}<span class="nulo">NULL</span></div>`;
}
function vEstructuras(){
  const niveles = [[3,'Persona con prestigio'],[2,'Amigo del gerente'],[1,'Cliente frecuente']].map(([n, t]) => `<div class="nivel"><div class="lblnv"><b>[${n}]</b> ${t}</div><div>${cadena(E.filaVip.filter(c => c.tarjeta === n))}</div></div>`).join('');
  const sel = mesaSel() || E.mesas.find(m => m.comanda.length) || null;
  const arr = Array.from({length: E.menuTamano}, (_, i) => E.menu[i]);
  const arrHtml = `<div class="slots" style="margin:22px 0 10px">${arr.map((p, i) => `<div class="slot ${p ? 'on' : ''}" style="width:92px"><span class="ix">${i}</span>${p ? `<b style="font-size:12.5px">${emoji(p.nombre)} ${h(p.nombre.length > 14 ? p.nombre.slice(0,13) + '…' : p.nombre)}</b>` : '<small>libre</small>'}</div>`).join('')}</div>`;
  return `<div class="dos">
    <div class="col">
      <div class="card"><h2>🔄 Cola circular · Meseros</h2><div class="sub">Arreglo de tamaño fijo (${E.meseros.capacidad}). Al llegar al final, <code>final = (final + 1) % capacidad</code> da la vuelta.</div>${ranuras(E.meseros)}<div class="nota">frente = ${E.meseros.frente} · final = ${E.meseros.final} · cantidad = ${E.meseros.cantidad}. Al asignar a un mesero sale del frente y vuelve a entrar por el final.</div></div>
      <div class="card"><h2>🔄 Cola circular · Domiciliarios</h2><div class="sub">Misma estructura, otra instancia (capacidad ${E.domiciliarios.capacidad}).</div>${ranuras(E.domiciliarios)}<div class="nota">frente = ${E.domiciliarios.frente} · final = ${E.domiciliarios.final} · cantidad = ${E.domiciliarios.cantidad}</div></div>
      <div class="card"><h2>📚 Pila · ${sel ? 'Comanda de la mesa ' + sel.id : 'Comandas'}</h2><div class="sub">Último en entrar, primero en salir (LIFO). Cada mesa tiene su propia pila.</div>
        ${sel && sel.comanda.length ? `<div class="apila">${sel.comanda.map((p, i) => ({p, i})).reverse().map(({p, i}) => `<div class="nodo" style="${i === sel.comanda.length - 1 ? 'border-color:var(--rojo);background:var(--suave)' : ''}">${emoji(p.nombre)} ${h(p.nombre)}${i === sel.comanda.length - 1 ? '<div class="lblc">TOPE ← sale primero</div>' : ''}</div>`).join('')}</div>` : '<div class="vacio">Ninguna mesa tiene productos todavía. Toma una comanda en el Salón.</div>'}</div>
    </div>
    <div class="col">
      <div class="card"><h2>➡️ Cola normal · Fila común</h2><div class="sub">Nodos enlazados: cada nodo guarda un cliente y apunta al siguiente (FIFO).</div>${cadena(E.filaComun)}</div>
      <div class="card"><h2>⭐ Cola con prioridad · Fila VIP</h2><div class="sub">Arreglo de colas: cada posición del arreglo es la cola de un nivel de tarjeta. Se atiende primero el nivel más alto con gente; dentro de cada nivel, por orden de llegada (FIFO).</div>${niveles}</div>
      <div class="card"><h2>🗂️ Arreglo dinámico · Menú</h2><div class="sub">${E.menu.length} plato(s) en un arreglo con espacio para ${E.menuTamano}. Si se llena, crece ×1.618 (razón áurea).</div>${arrHtml}</div>
    </div>
  </div>`;
}

function render(){
  if(!E) return;
  const mm = mesaSel();
  if(!mm){ const o = E.mesas.find(m => m.ocupada); ui.mesa = o ? o.id : null; }
  document.querySelectorAll('.tab').forEach(b => b.classList.toggle('on', b.dataset.t === ui.tab));
  $('#kv').textContent = money(E.ventas.total);
  const v = {salon: vSalon, domicilios: vDomicilios, admin: vAdmin, ventas: vVentas, estructuras: vEstructuras}[ui.tab];
  $('#vista').innerHTML = v();
}

// ---------- acciones ----------
function selMesa(id){
  const m = E.mesas.find(x => x.id === id);
  if(m && m.ocupada){ ui.mesa = id; render(); }
  else toast('La mesa ' + id + ' está libre. Llama a un cliente para ocuparla.', false);
}
function llamar(){ api('/api/llamar'); }
function agregarComanda(i){ api('/api/comanda', {mesa: ui.mesa, plato: i}); }
function deshacer(){ api('/api/deshacer', {mesa: ui.mesa}); }
function pagar(){ api('/api/pagar', {mesa: ui.mesa}); }
function domAgregar(i){ api('/api/domicilio/agregar', {plato: i}); }
function domDeshacer(){ api('/api/domicilio/deshacer'); }
async function domConfirmar(){
  const j = await api('/api/domicilio/confirmar', {cliente: ui.cli, direccion: ui.dir});
  if(j && j.ok){ ui.cli = ''; ui.dir = ''; render(); }
}
function liberar(tipo, id){ api('/api/personal/liberar', {tipo, id}); }
function agregarPersonal(tipo){ api('/api/personal/agregar', {tipo, nombre: ($('#np-' + tipo).value || '')}); }
function agregarPlato(){ api('/api/menu/agregar', {nombre: $('#pl-nombre').value, precio: $('#pl-precio').value}); }
function quitarPlato(){ ui.hallado = -1; api('/api/menu/quitar'); }
async function buscarPlato(){
  const j = await api('/api/menu/buscar', {nombre: $('#pl-buscar').value});
  ui.hallado = j && j.posicion !== undefined ? j.posicion : -1; render();
}
function agregarMesa(){ api('/api/mesas/agregar', {capacidad: $('#mesa-cap').value}); }

// ---------- ventanas ----------
function abrirRegistro(){
  ui.tarjeta = 0; ui.personas = 2;
  const tarjetas = [[0,'Sin tarjeta'],[1,'Cliente frecuente'],[2,'Amigo del gerente'],[3,'Persona con prestigio']];
  abrir(`<h3>➕ Registrar cliente</h3><div class="sub">Con tarjeta entra a la fila VIP; sin tarjeta, a la fila común.</div>
    <label class="lb">Nombre</label><input class="in" id="r-nombre" placeholder="Ej: Ana" onkeydown="if(event.key==='Enter')enviarRegistro()">
    <label class="lb">Cantidad de personas</label>
    <div class="step"><button onclick="cambiarPersonas(-1)">−</button><b id="r-pers">2</b><button onclick="cambiarPersonas(1)">+</button><span class="sub" style="margin:0">Mesa más grande: ${E.capacidadMaxima} personas</span></div>
    <label class="lb">Tarjeta de fidelización</label>
    <div class="chips">${tarjetas.map(([n, t]) => `<button class="chip ${n === 0 ? 'on' : ''}" data-n="${n}" onclick="elegirTarjeta(${n})">${n === 3 ? '⭐ ' : ''}${t}</button>`).join('')}</div>
    <div class="fila" style="margin-top:20px"><button class="btn s" onclick="cerrar()">Cancelar</button><button class="btn p" onclick="enviarRegistro()">Registrar</button></div>`);
  setTimeout(() => { const i = $('#r-nombre'); if(i) i.focus(); }, 50);
}
function cambiarPersonas(d){ ui.personas = Math.max(1, ui.personas + d); $('#r-pers').textContent = ui.personas; }
function elegirTarjeta(n){ ui.tarjeta = n; document.querySelectorAll('.chip').forEach(c => c.classList.toggle('on', Number(c.dataset.n) === n)); }
async function enviarRegistro(){
  const j = await api('/api/cliente', {nombre: $('#r-nombre').value, personas: ui.personas, tarjeta: ui.tarjeta});
  if(j && j.ok) cerrar();
}
function mostrarCuenta(c){
  const lineas = c.lineas.map(l => `<div class="ln"><span>${emoji(l.nombre)} ${h(l.nombre)}</span><span>${money(l.precio)}</span></div>`).join('');
  const extra = c.cliente ? `<div>Cliente: ${h(c.cliente)}</div><div>Dirección: ${h(c.direccion)}</div><div>Domiciliario: ${h(c.mesero)}</div>` : `<div>Atendió: ${h(c.mesero || '—')}</div>`;
  abrir(`<h3>🧾 Cuenta · ${h(c.titulo)}</h3><div class="sub">Productos del último al primero (se desapila la comanda)</div>
    <div class="ticket">${extra}<div style="margin:8px 0 4px">— — — — — — — — — —</div>${lineas}<div class="tt"><span>TOTAL</span><span>${money(c.total)}</span></div></div>
    <button class="btn v g" onclick="cerrar()">¡Gracias! Cerrar</button>`);
}

// ---------- inicio ----------
if(['salon','domicilios','admin','ventas','estructuras'].includes(location.hash.slice(1))) ui.tab = location.hash.slice(1);
api('/api/estado', null, 'GET');
</script>
</body>
</html>
)HTML";

#endif                    // Fin de la protección contra lecturas repetidas
