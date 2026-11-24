# TODO e limiti noti

Solo il lavoro ancora da fare e i limiti documentati di OGL; le voci risolte
vengono tolte.

## Da fare

**Funzionalità**

- Più luci per oggetto/modello: `glScene` le tiene già in un vettore, ma agli
  shader arriva solo `lights[0]`, che è anche l'unica a fare ombra.
- Render-to-texture.
- Gestione migliore dei tasti.

**Verifiche in sospeso**

- **Linux**: confermare che l'MSAA funzioni (l'hint `GLFW_SAMPLES` ora è
  richiesto anche lì), rieseguire `make test_asan` con `detect_leaks=1` e
  compilare con GCC.
- **Display HiDPI**: il viewport segue la dimensione del framebuffer in pixel;
  provato solo su un display a scala 1. Confermare su Retina ridimensionando
  una finestra.

## Limiti noti

- **Un solo thread.** Rendering, caricamento delle risorse e finestre vanno
  usati dal thread principale: lo stato interno non ha lock e GLFW stesso lo
  richiede.
- **Le finestre non condividono le risorse GPU.** Ogni `glWindow` ha il suo
  contesto: un oggetto disegnato in una seconda finestra viene ricaricato lì,
  e le risorse rimaste nel primo contesto sono liberate solo quando quel
  contesto viene distrutto. Lo stesso vale per un oggetto distrutto o
  re-inizializzato mentre è corrente un'altra finestra. Un oggetto alternato
  fra due finestre a ogni frame viene ricaricato a ogni frame: conviene dare a
  ogni finestra i suoi oggetti.
- **Texture e glifi restano in memoria fino alla fine del processo.** Le
  texture sono condivise per tipo + percorso del file, con una copia GPU per
  ogni finestra che le usa, e non vengono mai liberate dai materiali; lo
  stesso vale per l'atlante dei glifi. Caricare molte immagini diverse in una
  sessione lunga fa crescere la memoria GPU.
- **Il raggio di `glPoints` non è una misura nel mondo.** Lo shader calcola
  `gl_PointSize = raggio / distanza`: è il diametro in pixel a distanza 1
  dall'occhio. Per sfere di dimensione reale usare `glSphere`.
- **Stato OpenGL: `render()` imposta ciò che gli serve e non ripristina quasi
  nulla.** Vengono rimessi a posto solo blending, dimensione dei punti e
  depth mask. Il codice di rendering proprio deve impostare il suo stato prima
  di disegnare.
- **Le texture sono caricate sempre come RGB: il canale alfa si perde.** Una
  texture diffusa trasparente esce opaca, e una mappa di opacità funziona solo
  se l'opacità sta nei canali colore (`model.fs` legge il rosso). Per il
  supporto completo servono il caricamento con i canali originali e
  l'ordinamento da dietro in avanti degli oggetti trasparenti.
