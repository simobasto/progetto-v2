#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "generatore_di_mosse.hpp"

namespace scacchi {
// struct mosse;
enum class Colore { nessuno = 0, bianco = 8, nero = 16 };

enum class Tipo {
  vuoto = 0,
  pedone = 1,
  cavallo = 2,
  alfiere = 3,
  torre = 4,
  regina = 5,
  re = 6
};

struct Pezzo {
  Tipo tipo{Tipo::vuoto};
  Colore colore{Colore::nessuno};
};

struct movimento {
  Pezzo pezzo_di_partenza;
  Pezzo pezzo_in_arrivo;
  int partenza{-1};
  int arrivo{-1};

  // per identificare la tipologia di arrocco
  bool e_arrocco_corto = false;
  bool e_arrocco_lungo = false;

  // Backup dei diritti (servono per l'annullamento della mossa)
  bool vecchio_b_corto = true;
  bool vecchio_b_lungo = true;
  bool vecchio_n_corto = true;
  bool vecchio_n_lungo = true;

  // gestiamo la promozione
  bool promozione = false;

  // presa di en passant
  bool e_en_passant = false;
  int vecchia_en_passant{-1};
};

using ChiavePosizione = std::array<char, 72>;

class ListaMosse {
  std::array<movimento, 256> lista_;
  size_t conteggio_{0};

 public:
  // metodi per la dimensione
  bool empty() const { return conteggio_ == 0; }
  void clear() { conteggio_ = 0; }
  size_t size() const { return conteggio_; }
  void resize(size_t nuova_misura) { conteggio_ = nuova_misura; }

  // metodi per gli elementi
  auto begin() { return lista_.begin(); }
  auto end() { return lista_.begin() + conteggio_; }
  auto begin() const { return lista_.begin(); }
  auto end() const { return lista_.begin() + conteggio_; }

  void push_back(const movimento& m) {
    assert(conteggio_ < 256);  // non succede ma se succede...
    lista_[conteggio_] = m;
    ++conteggio_;
  }
  const movimento& operator[](size_t indice) const { return lista_[indice]; }
    ListaMosse& operator=(const ListaMosse& altra) {
    if (this != &altra) { //controllo che non siano la stessa lista
        conteggio_ = altra.conteggio_;
        // allineiamo la dimensione effettiva
        for (size_t i = 0; i < conteggio_; ++i) {
            lista_[i] = altra.lista_[i];
        }
    }
    return *this; //ritorna la lista modificata
}
};

class Board {
  std::array<Pezzo, 64> scacchiera_;
  Colore turno_;

  int pos_re_bianco_{4};
  int pos_re_nero_{60};

  bool arrocco_bianco_corto_{true};
  bool arrocco_bianco_lungo_{true};

  bool arrocco_nero_corto_{true};
  bool arrocco_nero_lungo_{true};

  int casella_en_passant_ = -1;

  int notazione_a_indice_(const std::string& casella) const;

  std::vector<ChiavePosizione> cronologia_posizioni_;

  ChiavePosizione ottieni_chiave_posizione_() const;

  void filtro_re_(int partenza, const mosse& m, ListaMosse&) const;
  void filtro_cavallo_(int partenza, const mosse& m, ListaMosse&) const;
  void filtro_torre_(int partenza, const mosse& m, ListaMosse&) const;
  void filtro_alfiere_(int partenza, const mosse& m, ListaMosse&) const;
  void filtro_regina_(int partenza, const mosse& m, ListaMosse&) const;

  void filtro_pedone_generalizzato_(int partenza,
                                    const ArrayMosse<2>& mosse_normali,
                                    const ArrayMosse<2>& catture,
                                    int riga_promozione, int riga_en_passant,
                                    ListaMosse& mosse_pseudo_legali)
      const;  // funzione di aiuto ai filtri pedone

  void filtro_pedone_nero_(int partenza, const mosse& m, ListaMosse&) const;
  void filtro_pedone_bianco_(int partenza, const mosse& m, ListaMosse&) const;

 public:
  bool e_triplice_ripetizione() const;
  // controlla che l'arrocco sia possibile
  bool puo_b_corto() const { return arrocco_bianco_corto_; }
  bool puo_b_lungo() const { return arrocco_bianco_lungo_; }

  bool puo_n_corto() const { return arrocco_nero_corto_; }
  bool puo_n_lungo() const { return arrocco_nero_lungo_; }

  Pezzo ottieni_pezzo(int casella) const {
    return scacchiera_[static_cast<size_t>(casella)];
  }
  Colore ottieni_turno() const { return turno_; }
  // funzioni della classe Board
  void ripristino();  // setta le condizioni iniziali della scacchiera
  void esegui_mossa(movimento& m);
  void annulla_mossa(movimento& m);
  bool re_sotto_scacco(Colore colore_re, const mosse& m) const;
  void ricerca_mosse_legali(const mosse& m,
                            ListaMosse& lista_mosse_da_riempire);

  bool esegui_mossa_da_notazione(const std::string& da, const std::string& a,
                                 const mosse& m);
  std::string indice_a_notazione(int indice) const;
  void stampa_scacchiera() const;
  int conta_pezzi() const;
  int conta_pezzi_maggiori() const;

   // METODO PUBBLICO USATO SOLO IN test.cpp PER TESTARE IL METODO PRIVATO
  int test_en_passant() const { return casella_en_passant_; }

};
}  // namespace scacchi