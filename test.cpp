#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "bot.hpp"
#include "doctest.h"
#include "generatore_di_mosse.hpp"
#include "partita.hpp"
#include "struttura.hpp"

using namespace scacchi;
//________________________________
// TEST SU GENERATORE_DI_MOSSE.CPP
//________________________________

TEST_CASE("Test modulo: generatore_di_mosse") {
  mosse m;

  SUBCASE("Test di geometria del re") {
    // 27 = d4 (il centro), il re ha 8 mosse
    REQUIRE(m.mosse_re[27].size() == 8);

    std::vector<int> attesi = {18, 19, 20, 26, 28, 34, 35, 36};
    for (const auto& ind : attesi) {
      // per ogni indice lo cerco nel vettore, se un indice non viene trovato
      // viene restituito un iteratore a .end(), quindi verifico che ciò non
      // avvenga
      CHECK(std::find(m.mosse_re[27].begin(), m.mosse_re[27].end(), ind) !=
            m.mosse_re[27].end());
    }
  }  // fine test geom. re

  SUBCASE("test di geometria cavallo") {
    // 27 = d4 (il centro), il cavallo ha 8 mosse
    REQUIRE(m.mosse_cavallo[27].size() == 8);
    std::vector<int> attesi = {10, 12, 17, 21, 33, 37, 42, 44};
    for (int ind : attesi) {
      CHECK(std::find(m.mosse_cavallo[27].begin(), m.mosse_cavallo[27].end(),
                      ind) != m.mosse_cavallo[27].end());
    }
    // test sull'angolo ([0] = a1), il cavallo ha 2 mosse
    REQUIRE(m.mosse_cavallo[0].size() == 2);
    CHECK((m.mosse_cavallo[0][0] == 10 || m.mosse_cavallo[0][0] == 17));
    CHECK((m.mosse_cavallo[0][1] == 10 || m.mosse_cavallo[0][1] == 17));
    CHECK(m.mosse_cavallo[0][0] != m.mosse_cavallo[0][1]);
  }  // fine test geom. cavallo

  SUBCASE("test di geometria torre, alfiere e regina") {
    // test torre [0] = a1, ha 14 mosse, 7 orizz. + 7 vert.
    int somma_mosse_torre =
        static_cast<int>(m.mosse_torre[0].direzioni[0].size() +
                         m.mosse_torre[0].direzioni[2].size() +
                         m.mosse_torre[0].direzioni[1].size() +
                         m.mosse_torre[0].direzioni[3].size());
    CHECK(somma_mosse_torre == 14);

    // test alfiere [0] = a1, ha 7 mosse (1 diag.)
    int somma_mosse_alfiere =
        static_cast<int>(m.mosse_alfiere[0].direzioni[0].size() +
                         m.mosse_alfiere[0].direzioni[1].size() +
                         m.mosse_alfiere[0].direzioni[2].size() +
                         m.mosse_alfiere[0].direzioni[3].size());
    CHECK(somma_mosse_alfiere == 7);

    // Testiamo la Regina in a1[0], ha le 14 mosse della torre + le 7
    // dell'alfiere
    int somma_mosse_regina =
        static_cast<int>(m.mosse_regina[0].direzioni[0].size() +
                         m.mosse_regina[0].direzioni[4].size() +
                         m.mosse_regina[0].direzioni[1].size() +
                         m.mosse_regina[0].direzioni[5].size() +
                         m.mosse_regina[0].direzioni[2].size() +
                         m.mosse_regina[0].direzioni[6].size() +
                         m.mosse_regina[0].direzioni[3].size() +
                         m.mosse_regina[0].direzioni[7].size());
    CHECK(somma_mosse_regina == 21);
  }  // fine test geom. torre, alfiere e regina

  SUBCASE("test geometrici sui pedoni, mossa_normale e cattura") {
    // PEDONE BIANCO, pos [27]d4, 1 mossa_norm. in d5[35], 2 prese in c5[34] e
    // e5[36]
    REQUIRE(m.mosse_pedone_bianco[27].mossa_normale.size() == 1);
    CHECK(m.mosse_pedone_bianco[27].mossa_normale[0] == 35);

    REQUIRE(m.mosse_pedone_bianco[27].cattura_del_pedone.size() == 2);
    CHECK((m.mosse_pedone_bianco[27].cattura_del_pedone[0] == 34 ||
           m.mosse_pedone_bianco[27].cattura_del_pedone[0] == 36));
    CHECK((m.mosse_pedone_bianco[27].cattura_del_pedone[1] == 34 ||
           m.mosse_pedone_bianco[27].cattura_del_pedone[1] == 36));

    // PEDONE NERO, pos [35]d5, 1 mossa_norm in d4[27], 2 prese in c4[26] e
    // e4[28]
    REQUIRE(m.mosse_pedone_nero[35].mossa_normale.size() == 1);
    CHECK(m.mosse_pedone_nero[35].mossa_normale[0] == 27);

    REQUIRE(m.mosse_pedone_nero[35].cattura_del_pedone.size() == 2);
    CHECK((m.mosse_pedone_nero[35].cattura_del_pedone[0] == 26 ||
           m.mosse_pedone_nero[35].cattura_del_pedone[0] == 28));
    CHECK((m.mosse_pedone_nero[35].cattura_del_pedone[1] == 26 ||
           m.mosse_pedone_nero[35].cattura_del_pedone[1] == 28));
    // PEDONE NERO doppio passo
    REQUIRE(m.mosse_pedone_nero[50].mossa_normale.size() == 2);
    CHECK(m.mosse_pedone_nero[50].mossa_normale[0] == 42);
    CHECK(m.mosse_pedone_nero[50].mossa_normale[1] == 34);

    // PEDONE BIANCO doppio passo
    REQUIRE(m.mosse_pedone_bianco[10].mossa_normale.size() == 2);
    CHECK(m.mosse_pedone_bianco[10].mossa_normale[0] == 18);
    CHECK(m.mosse_pedone_bianco[10].mossa_normale[1] == 26);
  }  // fine test geom. pedoni
}  // fine test su generatore_di_mosse.cpp

//______________________
// TEST SU STRUTTURA.HPP
//______________________

TEST_CASE("Test modulo: struttura") {
  Board b;
  mosse helper_mosse;

  SUBCASE("reset e posizione iniziale") {
    b.ripristino();
    // pos re Bianco
    CHECK((b.ottieni_pezzo(4).tipo == Tipo::re &&
           b.ottieni_pezzo(4).colore == Colore::bianco));
    // pos re Nero
    CHECK((b.ottieni_pezzo(60).tipo == Tipo::re &&
           b.ottieni_pezzo(60).colore == Colore::nero));

    // diritti di arrocco
    CHECK(b.puo_b_corto() == true);
    CHECK(b.puo_b_lungo() == true);
    CHECK(b.puo_n_corto() == true);
    CHECK(b.puo_n_lungo() == true);

    // una casella vuota a3[16]
    CHECK((b.ottieni_pezzo(16).tipo == Tipo::vuoto &&
           b.ottieni_pezzo(16).colore == Colore::nessuno));
  }  // fine test reset()

  SUBCASE("esegui e annulla mossa") {
    b.ripristino();
    // simulo un pedone bianco che si muova da e2 a e3
    Pezzo pedone_bianco{b.ottieni_pezzo(12)};
    Pezzo casella_vuota{b.ottieni_pezzo(20)};
    movimento mv{pedone_bianco, casella_vuota, 12, 20};
    b.esegui_mossa(mv);

    // controlliamo che la mossa sia stata eseguita bene
    CHECK(b.ottieni_turno() == Colore::nero);
    CHECK((b.ottieni_pezzo(20).tipo == Tipo::pedone &&
           b.ottieni_pezzo(20).colore == Colore::bianco));
    CHECK((b.ottieni_pezzo(12).tipo == Tipo::vuoto &&
           b.ottieni_pezzo(12).colore == Colore::nessuno));

    // i diritti di arrocco devono rimanere validi
    CHECK(b.puo_b_corto() == true);
    CHECK(b.puo_b_lungo() == true);
    CHECK(b.puo_n_corto() == true);
    CHECK(b.puo_n_lungo() == true);

    // annulliamo la mossa
    b.annulla_mossa(mv);
    CHECK(b.ottieni_turno() == Colore::bianco);
    CHECK((b.ottieni_pezzo(12).tipo == Tipo::pedone &&
           b.ottieni_pezzo(12).colore == Colore::bianco));
    CHECK((b.ottieni_pezzo(20).tipo == Tipo::vuoto &&
           b.ottieni_pezzo(20).colore == Colore::nessuno));

    CHECK(b.puo_b_corto() == true);
    CHECK(b.puo_b_lungo() == true);
    CHECK(b.puo_n_corto() == true);
    CHECK(b.puo_n_lungo() == true);
  }  // fine test esegui e annulla mossa

  SUBCASE("modifica diritti di arrocco, perdita e ripristino") {
    b.ripristino();
    Board b_iniziale;
    b_iniziale.ripristino();

    // usiamo una sequenza di 2 mosse
    Pezzo pedone_e2 = b.ottieni_pezzo(12);
    Pezzo vuota_e3 = b.ottieni_pezzo(20);
    movimento mv1{pedone_e2, vuota_e3, 12, 20};
    b.esegui_mossa(mv1);

    CHECK(b.puo_b_corto() == true);
    CHECK(b.puo_b_lungo() == true);

    // eseguiamo la mossa 2
    Pezzo re_e1 = b.ottieni_pezzo(4);
    Pezzo vuota_e2 = b.ottieni_pezzo(12);
    movimento mv2{re_e1, vuota_e2, 4, 12};
    b.esegui_mossa(mv2);

    CHECK(b.puo_b_corto() == false);
    CHECK(b.puo_b_lungo() == false);
    CHECK(b.puo_n_corto() == true);
    CHECK(b.puo_n_lungo() == true);

    b.annulla_mossa(mv2);

    CHECK(b.puo_b_corto() == true);
    CHECK(b.puo_b_lungo() == true);
    CHECK(b.puo_n_corto() == true);
    CHECK(b.puo_n_lungo() == true);

    b.annulla_mossa(mv1);
    CHECK(b.puo_b_corto() == true);
    CHECK(b.puo_b_lungo() == true);
    CHECK(b.puo_n_corto() == true);
    CHECK(b.puo_n_lungo() == true);

    // controlliamo che la scacchiera sia tornata uguale a dopo il reset
    for (int i = 0; i < 64; ++i) {
      CHECK(b.ottieni_pezzo(i).tipo == b_iniziale.ottieni_pezzo(i).tipo);
      CHECK(b.ottieni_pezzo(i).colore == b_iniziale.ottieni_pezzo(i).colore);
    }
  }  // fine test su diritto di arrocco e ripristino serie di mosse

  SUBCASE("test su cattura e ripristino pezzo") {
    b.ripristino();
    // mangiata virtuale
    Pezzo cavallo_bianco = b.ottieni_pezzo(1);
    Pezzo pedone_nero_bersaglio = b.ottieni_pezzo(50);

    REQUIRE((pedone_nero_bersaglio.tipo == Tipo::pedone &&
             pedone_nero_bersaglio.colore == Colore::nero));

    movimento mossa_cattura{cavallo_bianco, pedone_nero_bersaglio, 1, 50};
    b.esegui_mossa(mossa_cattura);

    CHECK((b.ottieni_pezzo(1).tipo == Tipo::vuoto &&
           b.ottieni_pezzo(1).colore == Colore::nessuno));
    CHECK((b.ottieni_pezzo(50).tipo == Tipo::cavallo &&
           b.ottieni_pezzo(50).colore == Colore::bianco));

    b.annulla_mossa(mossa_cattura);

    CHECK((b.ottieni_pezzo(1).tipo == Tipo::cavallo &&
           b.ottieni_pezzo(1).colore == Colore::bianco));
    CHECK((b.ottieni_pezzo(50).tipo == Tipo::pedone &&
           b.ottieni_pezzo(50).colore == Colore::nero));
  }  // fine test su cattura e ripristino pezzo

  SUBCASE("test promozione del pedone") {
    b.ripristino();
    Pezzo pedone_bianco{Tipo::pedone, Colore::bianco};
    Pezzo alfiere_nero =
        b.ottieni_pezzo(58);  // In 58 c'è l'alfiere nero iniziale (casella c8)
    movimento mossa_promozione{pedone_bianco, alfiere_nero, 49, 58};
    mossa_promozione.promozione = true;
    b.esegui_mossa(mossa_promozione);

    CHECK((b.ottieni_pezzo(49).tipo == Tipo::vuoto &&
           b.ottieni_pezzo(49).colore == Colore::nessuno));
    CHECK((b.ottieni_pezzo(58).tipo == Tipo::regina &&
           b.ottieni_pezzo(58).colore == Colore::bianco));

    b.annulla_mossa(mossa_promozione);

    CHECK((b.ottieni_pezzo(49).tipo == Tipo::pedone &&
           b.ottieni_pezzo(49).colore == Colore::bianco));
    CHECK((b.ottieni_pezzo(58).tipo == Tipo::alfiere &&
           b.ottieni_pezzo(58).colore == Colore::nero));
  }  // fine test su promozione

  SUBCASE("test su cronologia posizioni e simmetria") {
    b.ripristino();

    // All'inizio della partita non ci devono essere ripetizioni
    CHECK(b.e_triplice_ripetizione() == false);

    // Creiamo una mossa di esempio (es. pedone e2-e3)
    Pezzo pedone_bianco{b.ottieni_pezzo(12)};
    Pezzo casella_vuota{b.ottieni_pezzo(20)};
    movimento mv{pedone_bianco, casella_vuota, 12, 20};

    // Eseguiamo la mossa: la cronologia sale di 1 elemento internamente
    b.esegui_mossa(mv);

    // Annulliamo la mossa: pop_back() deve rimuovere esattamente quell'elemento
    b.annulla_mossa(mv);

    // Se la simmetria è corretta, lo stato torna pulito e
    // is_triplice_ripetizione non si attiva
    CHECK(b.e_triplice_ripetizione() == false);
  }

  SUBCASE("Test sui flag dell'en passant, la cattura e il ripristino") {
    b.ripristino();
    Pezzo pedone_e2{Tipo::pedone, Colore::bianco};
    Pezzo vuoto_e4{b.ottieni_pezzo(28)};
    Pezzo vuoto_e5{b.ottieni_pezzo(36)};
    movimento mv0{pedone_e2, vuoto_e4, 12, 28};

    Pezzo pedone_h7{b.ottieni_pezzo(54)};
    Pezzo vuoto_h6{b.ottieni_pezzo(46)};
    movimento mv1{pedone_h7, vuoto_h6, 54, 46};
    movimento mv2{pedone_e2, vuoto_e5, 28, 36};
    b.esegui_mossa(mv0);
    b.esegui_mossa(mv1);
    b.esegui_mossa(mv2);  // pedone bianco in posizione
    CHECK(b.ottieni_pezzo(36).tipo == Tipo::pedone);
    CHECK(b.ottieni_pezzo(36).colore == Colore::bianco);

    Pezzo pedone_d7{b.ottieni_pezzo(51)};
    Pezzo vuoto_d5{b.ottieni_pezzo(35)};
    movimento doppio_passo{pedone_d7, vuoto_d5, 51, 35};
    b.esegui_mossa(doppio_passo);  // pedone nero in posizione

    CHECK(b.test_en_passant() ==
          43);  // controllo che sia stata attivata la casella_en_passant_

    ListaMosse mosse_legali;
    b.ricerca_mosse_legali(helper_mosse, mosse_legali);

    bool ha_en_passant =
        std::ranges::any_of(mosse_legali, [](const movimento& mv) {
          return mv.partenza == 36 && mv.arrivo == 43 &&
                 mv.e_en_passant == true;
        });  // controllo che tra le mosse legali ci sia la presa di en_passant

    CHECK(ha_en_passant == true);
  }
}  // fine test struttura.hpp

//________________
// TEST SU BOT.HPP
//________________

TEST_CASE("Test modulo: bot") {
  Board b;
  Bot bot;
  mosse helper_mosse;

  SUBCASE("Valutazione Statica del Materiale") {
    b.ripristino();
    CHECK(bot.test_valuta_scacchiera(b) == 0);
  }  // fine valutazione statica

  SUBCASE("Determinazione profonditá ottimale") {
    b.ripristino();
    CHECK(bot.test_determina_profondita_ottimale_(b) == 4);
  }

  SUBCASE("Ricerca Mossa - Scelta della Mossa Migliore") {
    b.ripristino();

    scacchi::movimento mossa_scelta = bot.trova_mossa_migliore(b, helper_mosse);

    int casa_partenza = mossa_scelta.partenza;
    CHECK(b.ottieni_pezzo(casa_partenza).colore == scacchi::Colore::bianco);
    CHECK(b.ottieni_pezzo(casa_partenza).tipo != scacchi::Tipo::vuoto);
  }  // fine test modulo bot

  SUBCASE("Rilevamento della Triplice Ripetizione") {
    b.ripristino();
    Pezzo cavallo_bianco{Tipo::cavallo, Colore::bianco};
    Pezzo cavallo_nero{Tipo::cavallo, Colore::nero};
    Pezzo vuoto{Tipo::vuoto, Colore::nessuno};
    movimento b_avanti{cavallo_bianco, vuoto, 1, 18};
    movimento n_avanti{cavallo_nero, vuoto, 57, 42};

    // Creiamo le mosse di ritorno speculari
    movimento b_indietro{cavallo_bianco, vuoto, 18, 1};
    movimento n_indietro{cavallo_nero, vuoto, 42, 57};

    b.esegui_mossa(b_avanti);
    b.esegui_mossa(n_avanti);
    b.esegui_mossa(b_indietro);
    b.esegui_mossa(
        n_indietro);  // La posizione iniziale si ripete per la 2 volta
    CHECK(b.e_triplice_ripetizione() == false);

    b.esegui_mossa(b_avanti);
    b.esegui_mossa(n_avanti);
    b.esegui_mossa(b_indietro);
    b.esegui_mossa(n_indietro);

    b.esegui_mossa(b_avanti);
    b.esegui_mossa(n_avanti);
    b.esegui_mossa(b_indietro);
    b.esegui_mossa(n_indietro);

    CHECK(b.e_triplice_ripetizione() == true);
  }
}