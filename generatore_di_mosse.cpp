#include "generatore_di_mosse.hpp"

namespace scacchi {

bool e_dentro_scacchiera(int x, int y) {
  return x >= 0 && x < 8 && y >= 0 && y < 8;
}

void calcolatore_mosse_RE(mosse& m) {
  for (int i{0}; i < 64; ++i) {
    // scorro le caselle
    /* per gli spostamenti calcolo la posizione attuale e poi le variazioni che
    sono i dx e dy dove x indica l'ascissa mentre y l'ordinata, x va da 0 a 7
    mentre y va a salti e fa 0,8,16,24,32...etc per capirci  */
    int x{i % 8};  // calcolo la colonna in cui si trova il re
    int y{
        i /
        8};  // calcolo la riga, tronca il risultato, PARTE DA ZERO IL CONTEGGIO

    for (int dy{-1}; dy < 2; ++dy) {    // vado nella variazione delle righe
      for (int dx{-1}; dx < 2; ++dx) {  // vario sulle colonne

        if (dx != 0 || dy != 0) {  // calcolo la mossa solo se la posizione
                                   // finale è != da quella iniziale
          int nuova_x{x + dx};
          int nuova_y{y + dy};

          if (e_dentro_scacchiera(nuova_x, nuova_y)) {
            m.mosse_re[static_cast<size_t>(i)].push_back((nuova_y * 8) +
                                                         nuova_x);
          }
        }
      }
    }
  }
}

void calcolatore_mosse_CAVALLO(mosse& m) {
  // sono tutte le combinazioni possibili di mosse che può fare il cavallo

  const std::array<int, 8> dx = {2, 2, -2, -2, +1, -1, +1, -1};
  const std::array<int, 8> dy = {1, -1, 1, -1, 2, 2, -2, -2};

  for (int i{0}; i < 64; ++i) {
    // scorro le caselle
    int x{i % 8};
    int y{i / 8};

    for (size_t p{0}; p < 8; ++p) {
      int nuovo_x{x + dx[p]};
      int nuovo_y{y + dy[p]};

      if (e_dentro_scacchiera(nuovo_x, nuovo_y)) {
        m.mosse_cavallo[static_cast<size_t>(i)].push_back((nuovo_y * 8) +
                                                          nuovo_x);
      }
    }
  }
}

void calcolatore_mosse_TORRE_e_REGINA(mosse& m) {
  std::array<int, 4> dx = {1, -1, 0, 0};
  std::array<int, 4> dy = {0, 0, 1, -1};
  // set di mosse possibili della torre

  for (int i{0}; i < 64; ++i) {
    // scorro per le 64 caselle
    size_t indx = static_cast<size_t>(i);
    int x{i % 8};
    int y{i / 8};  // x e y li inizializzo qui cosí sono al sicuro nel ciclo

    for (size_t d{0}; d < 4; ++d) {
      // scorro gli array di direzione

      int nuovo_x{x + dx[d]};
      int nuovo_y{y + dy[d]};
      // evito sin da subito la casella di partenza

      while (e_dentro_scacchiera(nuovo_x, nuovo_y)) {
        int mossa{nuovo_y * 8 + nuovo_x};

        m.mosse_torre[indx].direzioni[d].push_back(mossa);
        m.mosse_regina[indx].direzioni[d].push_back(mossa);
        // salvo le mosse
        nuovo_x += dx[d];  // lo incremento per farlo uscire dal loop del while
        nuovo_y += dy[d];
        // finché il  pezzo dentro la scacchiera lo salvo
      }
    }
  }
}

void calcolatore_mosse_ALFIERE_e_REGINA(mosse& m) {
  std::array<int, 4> dx = {+1, -1, +1, -1};
  std::array<int, 4> dy = {+1, +1, -1, -1};
  // schema di mosse per l'alfiere

  for (int i{}; i < 64; ++i) {
    // per scorrere le caselle della scacchiera
    size_t indx = static_cast<size_t>(i);

    int x{i % 8};
    int y{i / 8};  // x e y li inizializzo fuori dal ciclo

    for (size_t d{0}; d < 4; ++d) {
      // le 4 direzioni

      int nuovo_x{x + dx[d]};
      int nuovo_y{y + dy[d]};

      while (e_dentro_scacchiera(nuovo_x, nuovo_y)) {
        int mossa{nuovo_x + (nuovo_y * 8)};
        m.mosse_alfiere[indx].direzioni[d].push_back(
            mossa);  // lo salvo nel vettore s
        m.mosse_regina[indx].direzioni[(d + 4)].push_back(mossa);
        // é d+4 per evitare di sovrascrivere le mosse giá salvate
        nuovo_x += dx[d];  // lo incremento per farlo uscire dal loop
        nuovo_y += dy[d];
        // finché la mossa rimane dentro la scacchiera lo salvo nel vettore
      }
    }
  }
}

/*ESSENDO CHE REGINA = TORRE + ALFIERE, LE SUE MOSSE SONO SALVATE SIA NELLA
 * PARTE DELLA TORRE CHE IN QUELLA DELL'ALFIERE */

void calcolatore_mosse_pedone_nero(mosse& m) {
  for (int i{0}; i < 64; ++i) {
    size_t indx = static_cast<size_t>(i);
    int x{i % 8};
    int y{i / 8};
    if (y > 0 && y <= 7) {  // sono le righe in cui si possono muovere
      m.mosse_pedone_nero[indx].mossa_normale.push_back(x + (y - 1) * 8);
      if (y == 6) {
        m.mosse_pedone_nero[indx].mossa_normale.push_back(x + (y - 2) * 8);
      }

      if (x >= 0 && x < 7) {  // mangiata verso DESTRA (guardando la scacchiera
                              // dalla parte dei bianchi)
        m.mosse_pedone_nero[indx].cattura_del_pedone.push_back((x + 1) +
                                                               ((y - 1) * 8));
      }
      if (x <= 7 && x > 0) {  // mangiata verso SINISTRA (guardando la
                             // scacchiera dalla parte dei bianchi)
        m.mosse_pedone_nero[indx].cattura_del_pedone.push_back((x - 1) +
                                                               ((y - 1) * 8));
      }
    }
  }
}

void calcolatore_mosse_pedone_bianco(mosse& m) {
  for (int i{0}; i < 64; ++i) {
    size_t indx = static_cast<size_t>(i);
    int x{i % 8};
    int y{i / 8};

    if (y >= 0 && y < 7) {  // singola mossa in avanti
      m.mosse_pedone_bianco[indx].mossa_normale.push_back(x + (y + 1) * 8);
      if (y == 1) {
        m.mosse_pedone_bianco[indx].mossa_normale.push_back(x + (y + 2) * 8);
      }

      if (x >= 0 && x < 7) {  // mangiata verso DESTRA (guardando la scacchiera
                              // dalla parte dei bianchi)
        m.mosse_pedone_bianco[indx].cattura_del_pedone.push_back((x + 1) +
                                                                 (y + 1) * 8);
      }
      if (x < 8 && x > 0) {  // mangiata verso SINISTRA (guardando la
                             // scacchiera dalla parte dei bianchi)
        m.mosse_pedone_bianco[indx].cattura_del_pedone.push_back((x - 1) +
                                                                 (y + 1) * 8);
      }
    }
  }
}

mosse::mosse() {
  calcolatore_mosse_RE(*this);
  calcolatore_mosse_CAVALLO(*this);
  calcolatore_mosse_TORRE_e_REGINA(*this);
  calcolatore_mosse_ALFIERE_e_REGINA(*this);
  calcolatore_mosse_pedone_nero(*this);
  calcolatore_mosse_pedone_bianco(*this);
}
}  // namespace scacchi