#include "struttura.hpp"

namespace scacchi {
//___________________________________
// IMPLEMENTAZIONE METODI DI BOARD
//___________________________________
int Board::notazione_a_indice_(const std::string& casella) const {
  // controllo che la mossa sia valida
  if (casella.length() != 2) {
    return -1;
  }
  // trasformiamo la mossa in ingresso nell'indice da passare alle altre parti
  int colonna{std::tolower(static_cast<unsigned char>(casella[0])) - 'a'};
  int riga{casella[1] - '1'};

  // controlliamo che sia dentro la scacchiera
  if (!e_dentro_scacchiera(colonna, riga)) {
    return -1;
  }

  return riga * 8 + colonna;
}

ChiavePosizione Board::ottieni_chiave_posizione_() const {
    ChiavePosizione chiave{};  // zero inizializzato
    chiave.fill('-'); //la riempio di trattini

    size_t pos{0};

    for (size_t i = 0; i < 64; ++i) {
        Pezzo pezzo = scacchiera_[i];
        if (pezzo.tipo == Tipo::vuoto) { chiave[pos++] = '.'; continue; }

        bool es_bianco = (pezzo.colore == Colore::bianco);
        switch (pezzo.tipo) {
            case Tipo::pedone:  chiave[pos++] = es_bianco ? 'P' : 'p'; break;
            case Tipo::cavallo: chiave[pos++] = es_bianco ? 'C' : 'c'; break;
            case Tipo::alfiere: chiave[pos++] = es_bianco ? 'A' : 'a'; break;
            case Tipo::torre:   chiave[pos++] = es_bianco ? 'T' : 't'; break;
            case Tipo::regina:  chiave[pos++] = es_bianco ? 'R' : 'r'; break;
            case Tipo::re:      chiave[pos++] = es_bianco ? 'K' : 'k'; break;
            default: break;
        }
    }

    chiave[pos] = (ottieni_turno() == Colore::bianco) ? 'W' : 'B';
    chiave[++pos] = arrocco_bianco_corto_ ? 'K' : '-';
    chiave[++pos] = arrocco_bianco_lungo_ ? 'Q' : '-';
    chiave[++pos] = arrocco_nero_corto_   ? 'k' : '-';
    chiave[++pos] = arrocco_nero_lungo_   ? 'q' : '-';

    // en passant — la casella in notazione occupa 2 char
    if (casella_en_passant_ == -1) {
        chiave[++pos] = '-';
    } else {
        auto nota = indice_a_notazione(casella_en_passant_);
        chiave[++pos] = nota[0];
        chiave[++pos] = nota[1];
    }

    return chiave;
}

bool Board::e_triplice_ripetizione() const {
  // se la cronologia é vuota non puó esserci ripetizione
  if (cronologia_posizioni_.empty()) return false;

  ChiavePosizione posizione_attuale = ottieni_chiave_posizione_();
int ripetizioni = static_cast<int>(
        std::ranges::count(cronologia_posizioni_, posizione_attuale));

  // controllo quante volte appare
  return (ripetizioni >= 3);
}

void Board::ripristino() {
  cronologia_posizioni_.clear();
  // svuotiamo tutta la scacchiera [nessuno,vuoto]
  std::ranges::fill(scacchiera_, Pezzo{Tipo::vuoto, Colore::nessuno});

  // mettiamo tutti i pedoni (8 - 15) e (48 - 55)
  std::ranges::fill(scacchiera_ | std::views::drop(8) | std::views::take(8),
                    Pezzo{Tipo::pedone, Colore::bianco});
  std::ranges::fill(scacchiera_ | std::views::drop(48) | std::views::take(8),
                    Pezzo{Tipo::pedone, Colore::nero});

  // schema T C A REGINA RE A C T
  // questi sono i pezzi pesanti bianchi
  scacchiera_[0] = scacchiera_[7] = {Tipo::torre, Colore::bianco};
  scacchiera_[1] = scacchiera_[6] = {Tipo::cavallo, Colore::bianco};
  scacchiera_[2] = scacchiera_[5] = {Tipo::alfiere, Colore::bianco};
  scacchiera_[3] = {Tipo::regina, Colore::bianco};
  scacchiera_[4] = {Tipo::re, Colore::bianco};

  // questi sono i pezzi pesanti neri
  scacchiera_[63] = scacchiera_[56] = {Tipo::torre, Colore::nero};
  scacchiera_[62] = scacchiera_[57] = {Tipo::cavallo, Colore::nero};
  scacchiera_[61] = scacchiera_[58] = {Tipo::alfiere, Colore::nero};
  scacchiera_[59] = {Tipo::regina, Colore::nero};
  scacchiera_[60] = {Tipo::re, Colore::nero};

  pos_re_bianco_ = 4;
  pos_re_nero_ = 60;

  // i diritti di arrocco a inizio partita ci sono
  arrocco_bianco_corto_ = true;
  arrocco_bianco_lungo_ = true;
  arrocco_nero_corto_ = true;
  arrocco_nero_lungo_ = true;

  // inizia con la mossa bianca
  turno_ = Colore::bianco;
  cronologia_posizioni_.push_back(ottieni_chiave_posizione_());
}

void Board::esegui_mossa(movimento& m) {
  // salvo lo stato del pezzo in arrivo per annulla_mossa
  m.pezzo_in_arrivo = scacchiera_[static_cast<size_t>(m.arrivo)];

  // si salvano i vecchi diritti sull'arrocco per poterli poi annullare dopo
  m.vecchio_b_corto = arrocco_bianco_corto_;
  m.vecchio_b_lungo = arrocco_bianco_lungo_;
  m.vecchio_n_corto = arrocco_nero_corto_;
  m.vecchio_n_lungo = arrocco_nero_lungo_;

  // aggiorno posizione en passant
  m.vecchia_en_passant = casella_en_passant_;  // salvo la pos attuale
  casella_en_passant_ = -1;
  int delta{m.arrivo - m.partenza};
  if (m.pezzo_di_partenza.tipo == Tipo::pedone &&
      (delta == 16 || delta == -16)) {
    casella_en_passant_ = (m.partenza + m.arrivo) / 2;
  }

  // aggiorna la posizione dei re quando si esegue la mossa
  if (m.pezzo_di_partenza.tipo == Tipo::re) {
    if (m.pezzo_di_partenza.colore == Colore::bianco) {
      pos_re_bianco_ = m.arrivo;

      // se muovo il re perdo entrambi i diritti di arrocco
      arrocco_bianco_corto_ = false;
      arrocco_bianco_lungo_ = false;
    }

    else {  // re nero
      pos_re_nero_ = m.arrivo;
      // se muovo il re perdo entrambi i diritti di arrocco
      arrocco_nero_corto_ = false;
      arrocco_nero_lungo_ = false;
    }
  }
  // controllo la torre, se si muove una torre perdo l'arrocco su quel lato
  if (m.partenza == 0) arrocco_bianco_lungo_ = false;
  if (m.partenza == 7) arrocco_bianco_corto_ = false;
  if (m.partenza == 56) arrocco_nero_lungo_ = false;
  if (m.partenza == 63) arrocco_nero_corto_ = false;

  // se una torre viene mangiata perdo comunque i diritti di arrocco
  if (m.arrivo == 0) arrocco_bianco_lungo_ = false;
  if (m.arrivo == 7) arrocco_bianco_corto_ = false;
  if (m.arrivo == 56) arrocco_nero_lungo_ = false;
  if (m.arrivo == 63) arrocco_nero_corto_ = false;

  // per eseguire l'arrocco
  if (m.e_arrocco_corto) {
    if (turno_ == Colore::bianco) {
      scacchiera_[6] = m.pezzo_di_partenza;             // Re in g1
      scacchiera_[5] = {Tipo::torre, Colore::bianco};   // Torre in f1
      scacchiera_[7] = {Tipo::vuoto, Colore::nessuno};  // Svuota h1
      pos_re_bianco_ = 6;  // aggiorno la posizione del re bianco

    } else {
      scacchiera_[62] = m.pezzo_di_partenza;             // Re in g8
      scacchiera_[61] = {Tipo::torre, Colore::nero};     // Torre in f8
      scacchiera_[63] = {Tipo::vuoto, Colore::nessuno};  // Svuota h8
      pos_re_nero_ = 62;  // aggiorno la posizione del re nero
    }
    scacchiera_[static_cast<size_t>(m.partenza)] = {
        Tipo::vuoto, Colore::nessuno};  // Svuota e1 (in ogni caso)

  } else if (m.e_arrocco_lungo) {
    if (turno_ == Colore::bianco) {
      scacchiera_[2] = m.pezzo_di_partenza;             // Re in c1
      scacchiera_[3] = {Tipo::torre, Colore::bianco};   // Torre in d1
      scacchiera_[0] = {Tipo::vuoto, Colore::nessuno};  // Svuota a1
      pos_re_bianco_ = 2;  // aggiorno la posizione del re bianco
    } else {
      scacchiera_[58] = m.pezzo_di_partenza;             // Re in c8
      scacchiera_[59] = {Tipo::torre, Colore::nero};     // Torre in d8
      scacchiera_[56] = {Tipo::vuoto, Colore::nessuno};  // Svuota a8
      pos_re_nero_ = 58;  // aggiorno la posizione del re nero
    }
    scacchiera_[static_cast<size_t>(m.partenza)] = {
        Tipo::vuoto, Colore::nessuno};  // svuota la casella del re in ogni caso

  } else {
    // se la mossa è una promozione
    if (m.promozione == true) {
      scacchiera_[static_cast<size_t>(m.arrivo)] = {Tipo::regina,
                                                    m.pezzo_di_partenza.colore};
    } else if (m.e_en_passant) {
      int casella_nemico =
          (m.pezzo_di_partenza.colore == Colore::bianco)
              ? m.arrivo - 8
              : m.arrivo + 8;  // casella dove effettivamente é il pedone
      scacchiera_[static_cast<size_t>(casella_nemico)] = {
          Tipo::vuoto, Colore::nessuno};  // lo tolgo
      scacchiera_[static_cast<size_t>(m.arrivo)] =
          m.pezzo_di_partenza;  // sposto il pedone nella casella di arrivo
      casella_en_passant_ = -1;

    } else {
      // se la mossa è normale
      scacchiera_[static_cast<size_t>(m.arrivo)] = m.pezzo_di_partenza;
    }
    scacchiera_[static_cast<size_t>(m.partenza)] = {
        Tipo::vuoto, Colore::nessuno};  // svuoto la partenza
  }
  
  turno_ = (turno_ == Colore::bianco) ? Colore::nero : Colore::bianco;
  cronologia_posizioni_.push_back(ottieni_chiave_posizione_());
}

void Board::annulla_mossa(movimento& m) {
  turno_ = (turno_ == Colore::bianco) ? Colore::nero : Colore::bianco;
  if (!cronologia_posizioni_.empty()) {
    cronologia_posizioni_.pop_back();
  }
  // Ripristina i diritti di arrocco memorizzati in esegui_mossa
  arrocco_bianco_corto_ = m.vecchio_b_corto;
  arrocco_bianco_lungo_ = m.vecchio_b_lungo;
  arrocco_nero_corto_ = m.vecchio_n_corto;
  arrocco_nero_lungo_ = m.vecchio_n_lungo;

  casella_en_passant_ = m.vecchia_en_passant;

  if (m.pezzo_di_partenza.tipo == Tipo::re) {
    if (m.pezzo_di_partenza.colore == Colore::bianco)
      pos_re_bianco_ = m.partenza;
    else
      pos_re_nero_ = m.partenza;
  }

  if (m.e_arrocco_corto) {
    if (m.pezzo_di_partenza.colore == Colore::bianco) {
      scacchiera_[4] = m.pezzo_di_partenza;            // Re torna in e1
      scacchiera_[7] = {Tipo::torre, Colore::bianco};  // Torre torna in h1
      scacchiera_[5] = scacchiera_[6] = {Tipo::vuoto, Colore::nessuno};
      pos_re_bianco_ = 4;  // agg. pos. re bianco

    } else {
      scacchiera_[60] = m.pezzo_di_partenza;          // Re torna in e8
      scacchiera_[63] = {Tipo::torre, Colore::nero};  // Torre torna in h8
      scacchiera_[61] = scacchiera_[62] = {Tipo::vuoto, Colore::nessuno};
      pos_re_nero_ = 60;  // agg. pos. re nero
    }
  } else if (m.e_arrocco_lungo) {
    if (m.pezzo_di_partenza.colore == Colore::bianco) {
      scacchiera_[4] = m.pezzo_di_partenza;            // Re torna in e1
      scacchiera_[0] = {Tipo::torre, Colore::bianco};  // Torre torna in a1
      scacchiera_[2] = scacchiera_[3] = {Tipo::vuoto, Colore::nessuno};
      pos_re_bianco_ = 4;  // agg. pos. re bianco
    } else {
      scacchiera_[60] = m.pezzo_di_partenza;          // Re torna in e8
      scacchiera_[56] = {Tipo::torre, Colore::nero};  // Torre torna in a8
      scacchiera_[58] = scacchiera_[59] = {Tipo::vuoto, Colore::nessuno};
      pos_re_nero_ = 60;  // agg. pos. re nero
    }
  } else if (m.e_en_passant) {
    // riporto il pedone
    scacchiera_[static_cast<size_t>(m.partenza)] = m.pezzo_di_partenza;
    scacchiera_[static_cast<size_t>(m.arrivo)] = {
        Tipo::vuoto, Colore::nessuno};  // svuoto la casa
    int casella_pedone_catturato =
        (m.pezzo_di_partenza.colore == Colore::bianco) ? m.arrivo - 8
                                                       : m.arrivo + 8;
    Colore colore_nemico = (m.pezzo_di_partenza.colore == Colore::bianco)
                               ? Colore::nero
                               : Colore::bianco;
    scacchiera_[static_cast<size_t>(casella_pedone_catturato)] = {
        Tipo::pedone, colore_nemico};
  } else {
    // Annullamento mossa normale
    scacchiera_[static_cast<size_t>(m.partenza)] = m.pezzo_di_partenza;
    scacchiera_[static_cast<size_t>(m.arrivo)] = m.pezzo_in_arrivo;
  }
}

bool Board::re_sotto_scacco(Colore colore_re, const mosse& m) const {

  int pos_re = (colore_re == Colore::bianco) ? pos_re_bianco_ : pos_re_nero_;
  Colore nemico = (colore_re == Colore::bianco) ? Colore::nero : Colore::bianco;
  size_t indx_pos_re{static_cast<size_t>(pos_re)};

  auto controllo_direzioni = [&](const auto& direzione, Tipo t1, Tipo t2) {
    for (const auto& d : direzione) {
      Pezzo p = scacchiera_[static_cast<size_t>(d)];
      if (p.colore == Colore::nessuno) continue;
      if (p.colore == colore_re) return false;
      if (p.colore == nemico) {
        return p.tipo == t1 || p.tipo == t2;
      }
    }
    return false;
  };

  // controllo su torre e regina
  if (std::ranges::any_of(
          m.mosse_torre[indx_pos_re].direzioni, [&](const auto& dir) {
            return controllo_direzioni(dir, Tipo::torre, Tipo::regina);
          })) {
    return true;
  }

  // controllo alfieri e regina
  if (std::ranges::any_of(
          m.mosse_alfiere[indx_pos_re].direzioni, [&](const auto& dir) {
            return controllo_direzioni(dir, Tipo::alfiere, Tipo::regina);
          })) {
    return true;
  }

  // controllo se dei cavalli sono "a tiro"
  for (const int& d : m.mosse_cavallo[indx_pos_re]) {
    Pezzo p = scacchiera_[static_cast<size_t>(d)];
    if (p.colore == nemico && p.tipo == Tipo::cavallo) return true;
  }

  // controllo i pedoni
  const auto& mosse_pedone =
      (colore_re == Colore::bianco)
          ? m.mosse_pedone_nero[indx_pos_re].cattura_del_pedone
          : m.mosse_pedone_bianco[indx_pos_re].cattura_del_pedone;

  for (const auto& d : mosse_pedone) {
    Pezzo p = scacchiera_[static_cast<size_t>(d)];
    if (p.colore == nemico && p.tipo == Tipo::pedone) return true;
  }

  // controllo anche che i re non stiano vicini
  for (const auto& d : m.mosse_re[indx_pos_re]) {
    Pezzo p = scacchiera_[static_cast<size_t>(d)];
    if (p.colore == nemico && p.tipo == Tipo::re) return true;
  }
  return false;
}

void Board::ricerca_mosse_legali(const mosse& m,
                                 ListaMosse& lista_mosse) {
  // Svuotiamo la lista
  lista_mosse.clear();
  ListaMosse mosse_pseudo_legali;

  auto filtro_tipo_di_pezzo = [&](int i) {
    size_t indx = static_cast<size_t>(i);
    if (scacchiera_[indx].tipo != Tipo::vuoto &&
        scacchiera_[indx].colore == turno_) {
      switch (scacchiera_[indx].tipo) {
        case Tipo::re:
          filtro_re_(i, m, mosse_pseudo_legali);
          break;
        case Tipo::cavallo:
          filtro_cavallo_(i, m, mosse_pseudo_legali);
          break;
        case Tipo::torre:
          filtro_torre_(i, m, mosse_pseudo_legali);
          break;
        case Tipo::alfiere:
          filtro_alfiere_(i, m, mosse_pseudo_legali);
          break;
        case Tipo::regina:
          filtro_regina_(i, m, mosse_pseudo_legali);
          break;
        default:
          if (turno_ == Colore::nero) {
            filtro_pedone_nero_(i, m, mosse_pseudo_legali);
          } else if (turno_ == Colore::bianco) {
            filtro_pedone_bianco_(i, m, mosse_pseudo_legali);
          };
          break;
      }
    }
  };
  std::ranges::for_each(std::views::iota(0, 64), filtro_tipo_di_pezzo);
  Colore colore_re{turno_};

  bool re_iniz_sottoscacco = re_sotto_scacco(colore_re, m);

  for (auto& mv : mosse_pseudo_legali) {
    // Controllo sul transito dell'arrocco
    if (mv.e_arrocco_corto || mv.e_arrocco_lungo) {
      if (re_iniz_sottoscacco) {
        continue;
      } else {
        int casa_transito =
            mv.e_arrocco_corto ? (mv.partenza + 1) : (mv.partenza - 1);

        // creo una mossa che sposta il re di 1 in direzione dell'arrocco
        movimento mossa_transito;
        mossa_transito.partenza = mv.partenza;
        mossa_transito.arrivo = casa_transito;
        mossa_transito.pezzo_di_partenza = mv.pezzo_di_partenza;
        mossa_transito.pezzo_in_arrivo = {Tipo::vuoto, Colore::nessuno};
        mossa_transito.promozione = false;
        mossa_transito.e_arrocco_corto = false;
        mossa_transito.e_arrocco_lungo = false;
        mossa_transito.e_en_passant = false;

        esegui_mossa(mossa_transito);
        bool transito_sotto_scacco{re_sotto_scacco(colore_re, m)};
        annulla_mossa(mossa_transito);

        if (transito_sotto_scacco == true) {
          continue;
        }
      }
    }

    esegui_mossa(mv);  // Simula

    if (!re_sotto_scacco(colore_re, m)) {  // Se il re è al sicuro...

      lista_mosse.push_back(mv);
    }
    annulla_mossa(mv);  // torno indietro
  }
}

bool Board::esegui_mossa_da_notazione(const std::string& da,
                                      const std::string& a, const mosse& m) {
  // controllo l'input
  int indice_partenza = notazione_a_indice_(da);
  int indice_arrivo = notazione_a_indice_(a);
  if (indice_arrivo < 0 || indice_partenza < 0) return false;

  // ricerca mossa
  ListaMosse lista;
  ricerca_mosse_legali(m, lista);

  auto it = std::ranges::find_if(lista, [&](const movimento& mov) {
    return mov.partenza == indice_partenza && mov.arrivo == indice_arrivo;
  });
  if (it != lista.end()) {
    esegui_mossa(*it);
    return true;
  }
  return false;
};

std::string Board::indice_a_notazione(int indice) const {
  int colonna = indice % 8;
  int riga = indice / 8;

  char lettera = static_cast<char>('a' + colonna);
  char numero = static_cast<char>('1' + riga);

  std::string risultato = "";
  risultato += lettera;
  risultato += numero;

  return risultato;
}

void Board::stampa_scacchiera() const {
  std::cout << "\n      a     b     c     d     e     f     g     h\n";
  std::cout << "   ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐\n";

  static constexpr std::array<std::array<const char*, 7>, 3> icone{{
      {" ", " ", " ", " ", " ", " ", " "},  // Nessun colore
      {" ", "♟", "♞", "♝", "♜", "♛", "♚"},  // Bianco
      {" ", "♙", "♘", "♗", "♖", "♕", "♔"}   // Nero
  }};

  for (int riga = 7; riga >= 0; --riga) {
    std::cout << (riga + 1) << "  │";  // Numero di riga a sinistra

    for (int col = 0; col < 8; ++col) {
      Pezzo p = ottieni_pezzo(riga * 8 + col);

      // Conversione dei valori enum in indici 
      size_t col_idx = static_cast<size_t>(p.colore) / 8;
      size_t tipo_idx = static_cast<size_t>(p.tipo);

      const char* icona = icone[col_idx][tipo_idx];

      std::cout << "  " << icona << "  │";
    }

    std::cout << "  " << (riga + 1) << "\n";  // Numero di riga a destra

    // Separatore tra le righe
    if (riga > 0) {
      std::cout << "   ├─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┤\n";
    }
  }

  std::cout << "   └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘\n";
  std::cout << "      a     b     c     d     e     f     g     h\n\n";
}

void Board::filtro_re_(int partenza, const mosse& m,
                       ListaMosse& mosse_pseudo_legali) const {
  assert(partenza >= 0 && partenza < 64);

  const Pezzo& pezzo_partenza{scacchiera_[static_cast<size_t>(partenza)]};
  std::ranges::for_each(m.mosse_re[static_cast<size_t>(partenza)], [&](int d) {
    const auto& pezzo_arrivo = scacchiera_[static_cast<size_t>(d)];

    if (pezzo_arrivo.colore != turno_) {
      movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
      mosse_pseudo_legali.push_back(mossa_buona);
    }
  });

  // ARROCCO BIANCO CORTO E LUNGO
  // arrocco corto
  if (turno_ == Colore::bianco && partenza == 4) {
    // per poter fare l'arrocco le caselle 5 e 6 (f1, g1) devono essere libere
    if (arrocco_bianco_corto_ && scacchiera_[5].tipo == Tipo::vuoto &&
        scacchiera_[6].tipo == Tipo::vuoto) {
      // le case sono vuote, genero la mossa
      movimento arr_corto{pezzo_partenza, {Tipo::vuoto, Colore::nessuno}, 4, 6};
      // mossa classificata come arrocco
      arr_corto.e_arrocco_corto = true;
      mosse_pseudo_legali.push_back(arr_corto);  // salvo la mossa
    }

    // arrocco lungo
    if (arrocco_bianco_lungo_ && scacchiera_[3].tipo == Tipo::vuoto &&
        scacchiera_[2].tipo == Tipo::vuoto &&
        scacchiera_[1].tipo == Tipo::vuoto) {

      movimento arr_lungo{pezzo_partenza, {Tipo::vuoto, Colore::nessuno}, 4, 2};
      // modifico l'indicatore bool
      arr_lungo.e_arrocco_lungo = true;
      mosse_pseudo_legali.push_back(arr_lungo);  // salvo la mossa
    }
  }

  //
  // ARROCCO NERO CORTO E LUNGO
  if (turno_ == Colore::nero && partenza == 60) {
    // per arroccare le caselle 61 e 62 (f8, g8) devono essere libere
    if (arrocco_nero_corto_ && scacchiera_[61].tipo == Tipo::vuoto &&
        scacchiera_[62].tipo == Tipo::vuoto) {
      // se soddisfo creo la mossa
      movimento arr_corto{
          pezzo_partenza, {Tipo::vuoto, Colore::nessuno}, 60, 62};
      arr_corto.e_arrocco_corto = true;
      mosse_pseudo_legali.push_back(arr_corto);  // salvo la mossa
    }

    // arrocco lungo
    if (arrocco_nero_lungo_ && scacchiera_[59].tipo == Tipo::vuoto &&
        scacchiera_[58].tipo == Tipo::vuoto &&
        scacchiera_[57].tipo == Tipo::vuoto) {
      // se soddisfo i requisiti "geometrici" genero la mossa arr_lungo
      movimento arr_lungo{
          pezzo_partenza, {Tipo::vuoto, Colore::nessuno}, 60, 58};
      // modifico l'indicatore bool
      arr_lungo.e_arrocco_lungo = true;
      mosse_pseudo_legali.push_back(arr_lungo);  // salvo la mossa
    }
  }
}

void Board::filtro_cavallo_(int partenza, const mosse& m,
                            ListaMosse& mosse_pseudo_legali) const {
  assert(partenza >= 0 && partenza < 64);
  const Pezzo& pezzo_partenza{scacchiera_[static_cast<size_t>(partenza)]};
  std::ranges::for_each(
      m.mosse_cavallo[static_cast<size_t>(partenza)],
      [&](int d) {
        const auto& pezzo_arrivo = scacchiera_[static_cast<size_t>(d)];

        if (pezzo_arrivo.colore != turno_) {
          movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
          mosse_pseudo_legali.push_back(mossa_buona);
        }
      }

  );
}

void Board::filtro_torre_(int partenza, const mosse& m,
                          ListaMosse& mosse_pseudo_legali) const {
  assert(partenza >= 0 && partenza < 64);
  const Pezzo& pezzo_partenza{scacchiera_[static_cast<size_t>(partenza)]};

  std::ranges::for_each(
      m.mosse_torre[static_cast<size_t>(partenza)].direzioni,
      [&](const auto& raggio_direzione) {
        for (int d : raggio_direzione) {
          const auto& pezzo_arrivo{scacchiera_[static_cast<size_t>(d)]};

          // casella vuota
          if (pezzo_arrivo.colore == Colore::nessuno) {
            movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
            mosse_pseudo_legali.push_back(mossa_buona);
          }

          // pezzo alleato
          else if (pezzo_arrivo.colore == turno_) {
            break;
          }

          // pezzo nemico
          else {
            movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
            mosse_pseudo_legali.push_back(mossa_buona);
            break;
          }
        }
      });
}

void Board::filtro_alfiere_(int partenza, const mosse& m,
                            ListaMosse& mosse_pseudo_legali) const {
  const Pezzo& pezzo_partenza{scacchiera_[static_cast<size_t>(partenza)]};

  std::ranges::for_each(
      m.mosse_alfiere[static_cast<size_t>(partenza)].direzioni,
      [&](const auto& raggio_direzione) {
        for (int d : raggio_direzione) {
          const auto& pezzo_arrivo{scacchiera_[static_cast<size_t>(d)]};

          // casella vuota
          if (pezzo_arrivo.colore == Colore::nessuno) {
            movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
            mosse_pseudo_legali.push_back(mossa_buona);
          }

          // pezzo alleato
          else if (pezzo_arrivo.colore == turno_) {
            break;
          }

          // pezzo nemico

          else {
            movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
            mosse_pseudo_legali.push_back(mossa_buona);
            break;
          }
        }
      });
}

void Board::filtro_regina_(int partenza, const mosse& m,
                           ListaMosse& mosse_pseudo_legali) const {
  filtro_torre_(partenza, m, mosse_pseudo_legali);
  filtro_alfiere_(partenza, m, mosse_pseudo_legali);
}

void Board::filtro_pedone_generalizzato_(
    int partenza, const ArrayMosse<2>& mosse_normali,
    const ArrayMosse<2>& catture, int riga_promozione, int riga_en_passant,
    ListaMosse& mosse_pseudo_legali) const {
  assert(partenza >= 0 && partenza < 64);
  const Pezzo& pezzo_partenza{scacchiera_[static_cast<size_t>(partenza)]};

  // valuto l'avanzamento
  for (const auto& d : mosse_normali) {
    const auto& pezzo_arrivo = scacchiera_[static_cast<size_t>(d)];

    if (pezzo_arrivo.tipo == Tipo::vuoto) {
      movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
      if (d / 8 == riga_promozione) {  // promozione
        mossa_buona.promozione = true;
      }
      mosse_pseudo_legali.push_back(mossa_buona);
    } else {
      break;
    }
  }

  // valuto la cattura
  for (const auto& d : catture) {
    const auto& pezzo_arrivo = scacchiera_[static_cast<size_t>(d)];

    Colore nemico = (turno_ == Colore::bianco) ? Colore::nero : Colore::bianco;
    if (pezzo_arrivo.colore == nemico) {
      movimento mossa_buona{pezzo_partenza, pezzo_arrivo, partenza, d};
      if (d / 8 == riga_promozione) {  // promozione
        mossa_buona.promozione = true;
      }
      mosse_pseudo_legali.push_back(mossa_buona);
    }
  }
  // valuto la presa di en passant
  if (partenza / 8 == riga_en_passant && casella_en_passant_ != -1) {
    if (std::ranges::find(catture, casella_en_passant_) != catture.end()) {
      // creiamo la mossa, l'arrivo è vuoto
      movimento mossa_ep{pezzo_partenza,
                         {Tipo::vuoto, Colore::nessuno},
                         partenza,
                         casella_en_passant_};
      mossa_ep.e_en_passant = true;  // attiviamo il flag
      // la aggiungo
      mosse_pseudo_legali.push_back(mossa_ep);
    }
  }
}

void Board::filtro_pedone_bianco_(int partenza, const mosse& m,
                                  ListaMosse& mosse_pseudo_legali) const {
  const auto& dati_partenza =
      m.mosse_pedone_bianco[static_cast<size_t>(partenza)];
  filtro_pedone_generalizzato_(partenza, dati_partenza.mossa_normale,
                               dati_partenza.cattura_del_pedone, 7, 4,
                               mosse_pseudo_legali);
}

void Board::filtro_pedone_nero_(int partenza, const mosse& m,
                                ListaMosse& mosse_pseudo_legali) const {
  const auto& dati_partenza =
      m.mosse_pedone_nero[static_cast<size_t>(partenza)];
  filtro_pedone_generalizzato_(partenza, dati_partenza.mossa_normale,
                               dati_partenza.cattura_del_pedone, 0, 3,
                               mosse_pseudo_legali);
}

int Board::conta_pezzi() const {
  return static_cast<int>(std::ranges::count_if(
      scacchiera_, [](const Pezzo& p) { return p.tipo != Tipo::vuoto; }));
}

int Board::conta_pezzi_maggiori() const {
  return static_cast<int>(
      std::ranges::count_if(scacchiera_, [](const Pezzo& p) {
        return p.tipo == Tipo::regina || p.tipo == Tipo::alfiere ||
               p.tipo == Tipo::cavallo || p.tipo == Tipo::torre;
      }));
}
}  // namespace scacchi