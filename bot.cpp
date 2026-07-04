#include "bot.hpp"

namespace scacchi {

int Bot::determina_profondita_ottimale_(const Board& b) const {
  int pezzi_rimasti{b.conta_pezzi()};

  if (pezzi_rimasti > 22) {
    return 4;  // inizio gioco
  } else if (pezzi_rimasti > 14) {
    return 5;  // inizio miedio gioco
  } else if (pezzi_rimasti > 8) {
    return 6;  // medio gioco puro
  } else {
    return 7;  // endgame
  }
}

int Bot::valuta_scacchiera_(const Board& b) const {
  int punteggio{0};

  bool e_endgame = (b.conta_pezzi_maggiori() <= 6);

  for (int i{0}; i < 64; ++i) {
    // ribalto la scacchiera se il colore è nero
    auto pezzo_corrente{b.ottieni_pezzo(i)};
    if (pezzo_corrente.tipo == Tipo::vuoto) {
      continue;
    }
    size_t indx = static_cast<size_t>(
        (pezzo_corrente.colore == Colore::nero) ? (i ^ 56) : i);

    // inizializzo le variabili per il calcolo
    int v_standard{0};
    int v_pos{0};

    switch (pezzo_corrente.tipo) {
      case Tipo::re:
        v_standard = 200000;
        if (e_endgame) {
          v_pos = pst_re_endgame[indx];
        } else {
          v_pos = pst_re[indx];
        }
        if (!e_endgame && (indx == 2 || indx == 6)) {
          v_pos += 40;
        }
        break;

      case Tipo::cavallo:
        v_standard = 300;
        v_pos = pst_cavallo[indx];
        break;

      case Tipo::torre:
        v_standard = 500;
        v_pos = pst_torre[indx];
        break;

      case Tipo::alfiere:
        v_standard = 300;
        v_pos = pst_alfiere[indx];
        break;

      case Tipo::regina:
        v_standard = 900;
        v_pos = pst_regina[indx];
        break;

      case Tipo::pedone: {
        v_standard = 100;
        v_pos = pst_pedone[indx];

        if (e_endgame) {
          int riga = static_cast<int>(indx) / 8;
          if (riga == 5) v_standard += 50;   // Sesta
          if (riga == 6) v_standard += 150;  // Settima
        } else {
          if (indx == 27 || indx == 28 || indx == 35 || indx == 36) {
            v_pos += 25;  // Bonus stabilità centrale
          }
        }
        break;
      }
      case Tipo::vuoto:
        break;
    }

    (pezzo_corrente.colore == Colore::bianco)
        ? (punteggio += v_standard + v_pos)
        : (punteggio -= (v_standard + v_pos));
  }
  return punteggio;
}

// assengna un punteggio alle mosse, per i tagli e l'ordinamento
int Bot::assegna_priorita_mossa_(const movimento& mossa) const {
  int priorita = 0;

  if (mossa.pezzo_in_arrivo.tipo != Tipo::vuoto) {
    priorita += 10000;

    // MVV: Diamo più punti se il pezzo mangiato è importante
    switch (mossa.pezzo_in_arrivo.tipo) {
      case Tipo::vuoto:
        break;
      case Tipo::regina:
        priorita += 5000;
        break;
      case Tipo::torre:
        priorita += 4000;
        break;
      case Tipo::alfiere:
        priorita += 3000;
        break;
      case Tipo::cavallo:
        priorita += 3000;
        break;
      case Tipo::pedone:
        priorita += 1000;
        break;
      default:
        break;
    }
    // LVA: Aggiungiamo un piccolo bonus se a colpire è un pezzo piccolo
    switch (mossa.pezzo_di_partenza.tipo) {
      case Tipo::vuoto:
        break;
      case Tipo::pedone:
        priorita += 50;
        break;
      case Tipo::cavallo:
        priorita += 40;
        break;
      case Tipo::alfiere:
        priorita += 30;
        break;
      case Tipo::torre:
        priorita += 20;
        break;
      case Tipo::regina:
        priorita += 10;
        break;
      default:
        break;
    }
  }

  if (mossa.promozione) {
    priorita += 9000;
  }
  if (mossa.e_arrocco_corto || mossa.e_arrocco_lungo) {
    priorita += 8000;
  }

  return priorita;
}

// ordina il vettore
void Bot::ordina_mosse_(ListaMosse& lista_mosse) const {
  // se c'è una sola mossa non c'è nulla da ordinare
  size_t dimensione = lista_mosse.size();
  if (dimensione <= 1) return;

  // Array di punteggi
  std::array<int, 256> punteggi;
  for (size_t i = 0; i < dimensione; ++i) {
    punteggi[i] = assegna_priorita_mossa_(lista_mosse[i]);
  }

  // array di indici
  std::array<size_t, 256> indici;
  for (size_t i = 0; i < dimensione; ++i) indici[i] = i;

  std::sort(
      indici.begin(), indici.begin() + dimensione,
      [&punteggi](size_t a, size_t b) { return punteggi[a] > punteggi[b]; });

  // Ricostruiamo la lista originale spostando i movimenti una sola volta
  ListaMosse lista_ordinata;
  for (size_t i = 0; i < dimensione; ++i) {
    lista_ordinata.push_back(lista_mosse[indici[i]]);
  }

  lista_mosse = lista_ordinata;
}

int Bot::alfa_beta_(Board& b, const mosse& m, int profondita, int alfa,
                    int beta, bool massimizza, int ply) const {
  // quiescence search
  if (profondita <= 0) {
    int punteggio_attuale = valuta_scacchiera_(b);

    if (massimizza) {
      if (punteggio_attuale >= beta) return punteggio_attuale; 
      alfa = std::max(alfa, punteggio_attuale);
    } else {
      if (punteggio_attuale <= alfa) return punteggio_attuale; 
      beta = std::min(beta, punteggio_attuale);
    }

    // generazione mosse
    ListaMosse mosse_legali_locali;
    b.ricerca_mosse_legali(m, mosse_legali_locali);

    // filtro di quiete
    // cancello le mosse che non catturano
    auto nuovo_end =
        std::ranges::remove_if(mosse_legali_locali, [](const movimento& mossa) {
          return mossa.pezzo_in_arrivo.tipo == Tipo::vuoto &&
                 !mossa.promozione && !mossa.e_en_passant;
        });

    size_t nuove_mosse_valide =
        static_cast<size_t>(nuovo_end.begin() - mosse_legali_locali.begin());
    mosse_legali_locali.resize(nuove_mosse_valide);

    if (mosse_legali_locali.empty()) {
      return punteggio_attuale;
    }

    // ordinamento mosse
    ordina_mosse_(mosse_legali_locali);

    if (massimizza) {
      int max_eval = punteggio_attuale;
      for (auto& mossa : mosse_legali_locali) {
        b.esegui_mossa(mossa);
        int eval = alfa_beta_(b, m, 0, alfa, beta, false, ply + 1);
        b.annulla_mossa(mossa);
        max_eval = std::max(max_eval, eval);
        alfa = std::max(alfa, eval);
        if (beta <= alfa) {
          break;  // Taglio Alfa
        }
      }
      return max_eval;
    } else {
      int min_eval = punteggio_attuale;
      for (auto& mossa : mosse_legali_locali) {
        b.esegui_mossa(mossa);
        int eval = alfa_beta_(b, m, 0, alfa, beta, true, ply + 1);
        b.annulla_mossa(mossa);
        min_eval = std::min(min_eval, eval);
        beta = std::min(beta, eval);
        if (beta <= alfa) {
          break;  // Taglio Beta
        }
      }
      return min_eval;
    }
  }  // fine if quiescence

  if (b.e_triplice_ripetizione())
    return 0;  // controllo la triplice ripetizione
  // generazione mosse
  ListaMosse mosse_legali_locali;
  b.ricerca_mosse_legali(m, mosse_legali_locali);

  // controllo matto e stallo
  // Ha senso farlo solo nella ricerca normale (profondita > 0)
  if (mosse_legali_locali.empty()) {
    if (b.re_sotto_scacco(b.ottieni_turno(), m)) {
      return massimizza ? (-10000000 + ply) : (10000000 - ply);
    } else {
      return 0;  // Stallo
    }
  }

  // ordinamento mosse
  ordina_mosse_(mosse_legali_locali);

  // ALFA-BETA
  if (massimizza) {
    int max_eval = std::numeric_limits<int>::min();

    for (auto& mossa : mosse_legali_locali) {
      b.esegui_mossa(mossa);
      int eval = alfa_beta_(b, m, profondita - 1, alfa, beta, false, ply + 1);
      b.annulla_mossa(mossa);

      max_eval = std::max(max_eval, eval);
      alfa = std::max(alfa, eval);

      if (beta <= alfa) {
        break;  // Taglio Alfa
      }
    }
    return max_eval;

  } else {
    int min_eval = std::numeric_limits<int>::max();

    for (auto& mossa : mosse_legali_locali) {
      b.esegui_mossa(mossa);
      int eval = alfa_beta_(b, m, profondita - 1, alfa, beta, true, ply + 1);
      b.annulla_mossa(mossa);

      min_eval = std::min(min_eval, eval);
      beta = std::min(beta, eval);

      if (beta <= alfa) {
        break;  // Taglio Beta
      }
    }
    return min_eval;
  }
}

// algoritmo di scelta della mossa
movimento Bot::trova_mossa_migliore(Board& b, const mosse& m) {
  ListaMosse mosse_legali;
  b.ricerca_mosse_legali(m, mosse_legali);

  if (mosse_legali.empty()) return movimento();

  int profondita_massima = determina_profondita_ottimale_(b);
  bool sono_bianco = (b.ottieni_turno() == Colore::bianco);

  movimento mossa_migliore = *mosse_legali.begin();

  // iteriamo da profondita 1 fino alla massima
  for (int profondita = 1; profondita <= profondita_massima; ++profondita) {
    // organizziamo le mosse anche sulle varie profonditá
    ordina_mosse_(mosse_legali);

    if (profondita > 1) {
      auto it = std::ranges::find_if(mosse_legali, [&](const movimento& mv) {
        return mv.partenza == mossa_migliore.partenza &&
               mv.arrivo == mossa_migliore.arrivo;
      });
      if (it != mosse_legali.end() && it != mosse_legali.begin()) {
        // sposta la mossa migliore in testa
        std::rotate(mosse_legali.begin(), it, it + 1);
      }
    }

    int alfa = std::numeric_limits<int>::min();
    int beta = std::numeric_limits<int>::max();

    int punteggio_max{sono_bianco ? std::numeric_limits<int>::min()
                                  : std::numeric_limits<int>::max()};
    ListaMosse mosse_ottime;

    for (auto& mossa : mosse_legali) {
      b.esegui_mossa(mossa);

      int eval = alfa_beta_(b, m, profondita - 1, alfa, beta, !sono_bianco, 1);
      b.annulla_mossa(mossa);

      if (sono_bianco) {
        if (eval > punteggio_max) {
          punteggio_max = eval;
          mosse_ottime.clear();
          mosse_ottime.push_back(mossa);
          mossa_migliore = mossa;  // aggiorna la migliore
          alfa = std::max(alfa, punteggio_max); // Restringe alfa
        } else if (eval == punteggio_max) {
          mosse_ottime.push_back(mossa);
        }
      } else {
        if (eval < punteggio_max) {
          punteggio_max = eval;
          mosse_ottime.clear();
          mosse_ottime.push_back(mossa);
          mossa_migliore = mossa;  // aggiorna la migliore
          beta = std::min(beta, punteggio_max); // Restringe beta
        } else if (eval == punteggio_max) {
          mosse_ottime.push_back(mossa);
        }
      }
    }

    if (!mosse_ottime.empty()) {
      mossa_migliore =
          mosse_ottime[static_cast<size_t>(gen_()) % mosse_ottime.size()];
    }
  }

  return mossa_migliore;
}

}  // namespace scacchi