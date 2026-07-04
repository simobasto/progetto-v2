#include "partita.hpp"

namespace scacchi {
// inizializziamo la scacchiera
Gioco::Gioco() { b_.ripristino(); }

void Gioco::seleziona_modalita_() {
  std::cout << "Seleziona la modalita' di gioco:\n";
  std::cout << " [1] Umano vs Bot \n";
  std::cout << " [2] Bot vs Bot (spettatore) \n";
  std::cout << " [3] Umano vs Umano \n";
  std::cout << "Scelta: ";

  std::cin >> modalita_;

  if (std::cin.fail() || (modalita_ != 1 && modalita_ != 2 && modalita_ != 3)) {
    std::cerr << "MODALITÀ NON VALIDA!\n";
    partita_in_corso_ = false;
    return;
  }
}  // seleziona_modalita_

void Gioco::controllo_fine_partita_(const ListaMosse& mosse_disponibili) {
  if (mosse_disponibili.empty()) {
    // se non ci sono mosse la partita é finita
    partita_in_corso_ = false;

    // stampiamo la scacchiera in assetto finale
    std::system("clear");
    b_.stampa_scacchiera();

    if (b_.re_sotto_scacco(b_.ottieni_turno(), m_)) {
      std::cout << "SCACCO MATTO! ";
      std::cout << (b_.ottieni_turno() == Colore::bianco
                        ? "HA VINTO IL NERO!\n"
                        : "HA VINTO IL BIANCO!\n");
      return;
    } else {
      std::cout << "STALLO" << "\n";
      return;
    }
  }
}  // controllo_fine_partita_
void Gioco::umano_turno_generalizzato_(long int& tempo_rimasto,
                                       std::string colore) {
  // per la gestione di input errati
  const int tentativi_max{5};
  int tentativi_falliti = 0;
  bool mossa_eseguita{false};

  auto inizio_turno = std::chrono::high_resolution_clock::now();
  while (!mossa_eseguita) {
    std::system("clear");
    b_.stampa_scacchiera();

    if (tentativi_falliti > 0) {
      std::cout << "Mossa impossibile, riprova! \n";
      std::cout << "Tentativi rimasti: " << (tentativi_max - tentativi_falliti)
                << "/" << tentativi_max << "\n\n";
    }

    std::string da{};
    std::string a{};

    std::cout << "Inserisci mossa ";
    std::cin >> da;
    std::cin >> a;

    if (!b_.esegui_mossa_da_notazione(da, a, m_)) {
      ++tentativi_falliti;
      std::cout << "mossa impossibile, riprova \n";
      std::cout << "tentativi rimasti: " << (tentativi_max - tentativi_falliti)
                << "/" << tentativi_max << "\n";
      if (tentativi_falliti >= tentativi_max) {
        std::cerr << "TROPPI ERRORI CONSECUTIVI \n";
        partita_in_corso_ = false;
        return;
      }
    } else {
      auto fine_turno =
          std::chrono::high_resolution_clock::now();  // fermo il timer
      mossa_eseguita = true;

      auto secondi_passati = std::chrono::duration_cast<std::chrono::seconds>(
                                 fine_turno - inizio_turno)
                                 .count();
      tempo_rimasto -= secondi_passati;
      std::string vincitore = (colore == "BIANCO") ? "NERO" : "BIANCO";
      if (tempo_rimasto <= 0) {
        std::cout << "TEMPO SCADUTO, VINCE IL " << vincitore << "\n";

        partita_in_corso_ = false;
        return;
      }  // fine if tempo scaduto
      std::cout << "Tempo residuo = " << tempo_rimasto << " s \n";
    }
  }
}  // umano_turno_generalizzato_
void Gioco::umano_turno_bianco_() {
  umano_turno_generalizzato_(tempo_bianco_, "BIANCO");
}  // umano_turno_bianco_

void Gioco::umano_turno_nero_() {
  umano_turno_generalizzato_(tempo_nero_, "NERO");
}  // umano_turno_nero_

void Gioco::bot_turno_generalizzato_(long int& tempo_rimasto,
                                     std::string colore) {
  std::system("clear");
  b_.stampa_scacchiera();

  std::cout << "il BOT  " << colore << " sta pensando... \n";

  auto inizio_turno = std::chrono::high_resolution_clock::now();
  auto mossa_bot = bot_.trova_mossa_migliore(b_, m_);
  auto fine_turno = std::chrono::high_resolution_clock::now();

  auto ms_passati = std::chrono::duration_cast<std::chrono::milliseconds>(
                        fine_turno - inizio_turno)
                        .count();
  auto secondi_passati = std::chrono::duration_cast<std::chrono::seconds>(
                             fine_turno - inizio_turno)
                             .count();

  tempo_rimasto -= secondi_passati;

  if (tempo_rimasto <= 0) {
    std::string vincitore = (colore == "BIANCO") ? "NERO" : "BIANCO";
    std::cout << "TEMPO SCADUTO, VINCE IL " << vincitore << "\n";
    partita_in_corso_ = false;
    return;
  }  // controllo sul tempo

  b_.esegui_mossa(mossa_bot);

  std::cout << "mossa bot " << colore << " da "
            << b_.indice_a_notazione(mossa_bot.partenza) << " a "
            << b_.indice_a_notazione(mossa_bot.arrivo) << "\n";
  std::cout << "Tempo residuo = " << tempo_rimasto << " s \n";
  std::cout << "Tempo per la mossa = " << ms_passati << " ms \n";
  std::this_thread::sleep_for(
      std::chrono::seconds(1));  // stoppiamo per far vedere
}  // bot_turno_generalizzato_

void Gioco::bot_bianco_() {
  bot_turno_generalizzato_(tempo_bianco_, "BIANCO");
}  // bot_turno_bianco_

void Gioco::bot_nero_() {
  bot_turno_generalizzato_(tempo_nero_, "NERO");
}  // bot_turno_nero_

void Gioco::giochiamo() {
  seleziona_modalita_();

  while (partita_in_corso_) {
    if (b_.e_triplice_ripetizione()) {
      std::cout << "PATTA PER TRIPLICE RIPETIZIONE!\n";
      b_.stampa_scacchiera();
      partita_in_corso_ = false;
      break;
    }

    scacchi::ListaMosse mosse_disponibili;
    b_.ricerca_mosse_legali(m_, mosse_disponibili);
    controllo_fine_partita_(mosse_disponibili);

    if (!partita_in_corso_) break;

    if (b_.ottieni_turno() == Colore::bianco) {
      if (modalita_ == 1 || modalita_ == 3) {
        umano_turno_bianco_();
      } else if (modalita_ == 2) {
        bot_bianco_();
      }
    } else if (b_.ottieni_turno() == Colore::nero) {
      if (modalita_ == 1 || modalita_ == 2) {
        bot_nero_();
      } else if (modalita_ == 3) {
        umano_turno_nero_();
      }
    }
  }
}

}  // namespace scacchi