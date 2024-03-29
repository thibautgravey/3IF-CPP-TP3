/*************************************************************************
                           Catalogue  -  description
                             -------------------
    début                : 19 NOVEMBRE 2019
    copyright            : (C) 2019 par BRANCHEREAU, GRAVEY
    e-mail               : $EMAIL$
*************************************************************************/

//---------- Réalisation de la classe <Catalogue> (fichier Catalogue.cpp) ------------

//---------------------------------------------------------------- INCLUDE

//-------------------------------------------------------- Include système
using namespace std;
#include <iostream>
#include <cstring>
#include <sstream>
#include <string>

//------------------------------------------------------ Include personnel
#include "Catalogue.h"
#include "ListeTrajets.h"

//------------------------------------------------------------- Constantes

static const int TAILLE_ENTREE_VILLE = 45;
static const int TAILLE_ENTREE_MOYEN_TRANSPORT = 20;

//----------------------------------------------------------------- PUBLIC

//----------------------------------------------------- Méthodes publiques

int Catalogue::Sauvegarde(string cheminAcces, CritereSelection critere)
// Algorithme :
// L'ouverture du fichier peut se faire en écriture simple (reset du fichier à son ouverture et écriture au début)
// ou en mode "append" où l'on ne réinitialise pas le fichier, mais on écrit simplement à la suite de l'existant
//
// Rien de particulier ici, le gros du travail réside dans les fonctions askParameters, respectCriteria, createBuffer
// et enfin putInFile.
//
// param1, param2 et param3 n'ont pas de nom explicites car ils diffèrent d'utilisation selon le critère choisi.
// Le matching des paramètres est décrit dans l'en tête de la fonction respectCriteria dans l'interface Catalogue.h.
{
    //ofstream fichierDeSortie(cheminAcces,fstream::app);
    string out;

    cout << "Souhaitez vous écraser les données présentes si le fichier existe déjà ? (Y/N)" << endl;
    cin >> out;

    while(cin.fail() || !(out == "Y" || out == "y" || out == "N" || out == "n"))
    {
        cin.clear();
        cin.ignore(10000,'\n');

        cout << "Mauvaise saisie. Veuillez recommencer." << endl;
        cin >> out;
    }

    ofstream fichierDeSortie(cheminAcces,((out=="Y" || out == "y") ? (fstream::out) : (fstream::app)));

    ostringstream buffer;

    //cout << "Sauvegarde" << endl;

    if(!fichierDeSortie)
    {
        cerr << "Une erreur s'est produite lors de l'ouverture du fichier. Abandon de la sauvegarde." << endl;
        return -1;
    }

    cout << "Fichier bien ouvert." << endl;

    //Critères d'entrées du filtre de sélection
    string param1;
    string param2;
    int param3(0);

    // on relève les paramètres donnés par l'utilisateur, en fonction du critère
    askParameters(critere,param1,param2);

    //Remise à l'échelle vis à vis du catalogue si besoin en est.
    if(critere == TRAJETS && stoi(param2)>listeTraj.GetNbTrajets())
    {
        cerr << "Borne supérieure trop haute, mise à la taille maximale du catalogue automatique." << endl;
        param2=to_string(listeTraj.GetNbTrajets());
    }

    int countSave(0);

    //Passage du filtre et mise dans le fichier
    for(int i = 0; i<listeTraj.GetNbTrajets(); i++)
    {
        ++param3;
        if(respectCriteria(listeTraj.GetListe()[i],critere,param1,param2,param3))
        {
            createBuffer(buffer,listeTraj.GetListe()[i]);
            if(putInFile(fichierDeSortie,countSave,buffer)==-1)
            {
                cerr << "Erreur de la sauvegarde dans le fichier." << endl;
                return -1;
            }
        }
    }

    cout << endl << "---- " << countSave << " Trajet(s) correctement sauvegardée(s)---- " << endl;

    fichierDeSortie.close();

    return 0;

} //----- Fin de Sauvegarde

int Catalogue::Charge(string cheminAcces,CritereSelection critere)
// Algorithme :
// Lit chaque ligne du fichier passé en paramètre et construit un trajet à partir des informations lues.
// Ce trajet est alors ajouté au catalogue si il respecte le critère indiqué, sinon il est détruit.
//
// La valeur de retour indique si le chargement a bien fonctionné (0) ou si il y a eu une erreur (-1).
{
    ifstream fichierEntree(cheminAcces);

    cout<<"Chargement de trajets"<<endl;

    if(!fichierEntree.is_open())
    {
        cerr << "Une erreur s'est produite lors de l'ouverture du fichier. Abandon" << endl;
        return -1;
    }

    cout << "Fichier bien ouvert" << endl;

    //Critères d'entrées du filtre
    string param1;
    string param2;
    int param3(0);

    // on relève les paramètres donnés par l'utilisateur, en fonction du critère
    askParameters(critere,param1,param2);

    int countCharge(0);
    Trajet* ptr_trajet = nullptr;

    //Passage du filtre de sélection et chargement du catalogue
    while(fichierEntree)
    {
        ptr_trajet = construitTrajetAvecLecture(fichierEntree);
        if(ptr_trajet!=nullptr)
        {
            ++param3;
            // on regarde si le trajet considéré remplit le critère, en fonction des paramètres que l'utilisateur a fournis
            if(respectCriteria(ptr_trajet,critere,param1,param2,param3))
            {
                listeTraj.AddTrajet(ptr_trajet);
                ++countCharge;
            }
            else
            {
                delete ptr_trajet;
            }
        }
    }

    cout << endl << "---- " << countCharge << " Trajet(s) correctement chargé(s)---- " << endl;

    fichierEntree.close();

    return 0;
} //----- Fin de Charge


void Catalogue::AddTrajetSimple()
// Algorithme :
// On demande à l'utilisateur les villes de départ , d'arrivée et le moyen de transport
// à l'aide des trois méthodes ask... et on crée ensuite un trajet Simple que l'on ajoute
// dans l'attribut listTraj (qui est une liste de trajets)
{
    char * vDepart = askVilleDepart();
    char * vArrivee = askVilleArrivee();
    char * vMoyenTransport = askMoyenTransport();
    TrajetSimple * nouveauTrajet = new TrajetSimple(vDepart,vArrivee,vMoyenTransport);
    listeTraj.AddTrajet(nouveauTrajet);
    delete [] vDepart;
    delete [] vArrivee;
    delete [] vMoyenTransport;
} //----- Fin de addTrajetSimple

void Catalogue::AddTrajetCompose()
// Algorithme :
// On demande à l'utilisateur les villes de départ et d'arrivée globales du trajet, ainsi que
// le nombre de sous trajets.
// On s'assure que les entrées sont correctes à l'aide de While, et on demande de les resaisir tant qu'elles
// ne le sont pas.
// on crée des variables temporaire à chaque nouvelle étape,  pour stocker les villes et le moyen de transport, que l'on
// détruit une fois les trajets crées.
{
    int nbTrajets;
    ListeTrajets* liste=new ListeTrajets();
    cout<<"Nombre de trajets ?"<<endl;
    cin >> nbTrajets;
    cin.ignore();
    while(cin.fail())
    {
        cerr << "Erreur, veuillez entrer un entier" <<endl;
        cin.clear();          //on clear les erreurs
        cin.ignore(10000,'\n');         // on ignore la prochaine ligne
        cin >> nbTrajets;             //on redemande la valeur;
        cin.ignore();
    }

    char vDepartPrincipale[TAILLE_ENTREE_VILLE+1];
    char vArriveePrincipale[TAILLE_ENTREE_VILLE+1];
    char vArriveePrecedent[TAILLE_ENTREE_VILLE+1];

    char * vDepart = askVilleDepart();
    char * vArrivee = askVilleArrivee();
    char * mTransport = askMoyenTransport();

    TrajetSimple* tmpTrajet=new TrajetSimple(vDepart,vArrivee,mTransport);

    liste->AddTrajet(tmpTrajet);
    strcpy(vArriveePrecedent,vArrivee);
    strcpy(vDepartPrincipale,vDepart);

    for(int i=1;i<nbTrajets;i++)
    {
        delete [] vArrivee;
        delete [] vDepart;
        delete [] mTransport;
        vDepart = askVilleDepart();
        vArrivee = askVilleArrivee();
        mTransport = askMoyenTransport();

        if(strcmp(vDepart,vArriveePrecedent)==0)
        {
            TrajetSimple* tmpTrajet=new TrajetSimple(vDepart,vArrivee,mTransport);
            liste->AddTrajet(tmpTrajet);
            strcpy(vArriveePrecedent,vArrivee);
        }
        else
        {
            while(strcmp(vDepart,vArriveePrecedent)!=0)
            {
                cerr << "Erreur, vous devez repartir de la ville d'étape précédente" <<endl;
                delete [] vArrivee;
                delete [] vDepart;
                delete [] mTransport;
                vDepart = askVilleDepart();
                vArrivee = askVilleArrivee();
                mTransport = askMoyenTransport();
            }

            TrajetSimple* tmpTrajet=new TrajetSimple(vDepart,vArrivee,mTransport);
            liste->AddTrajet(tmpTrajet);
            strcpy(vArriveePrecedent,vArrivee);
        }
    }

    strcpy(vArriveePrincipale,vArrivee);

    TrajetCompose* nouveauTrajetCompose=new TrajetCompose(vDepartPrincipale,vArriveePrincipale,liste);
    listeTraj.AddTrajet(nouveauTrajetCompose);

    delete [] vArrivee;
    delete [] vDepart;
    delete [] mTransport;
} //----- Fin de AddTrajetCompose



void Catalogue::RechercheSimple()
// Algorithme :
// On récupère la liste de trajets en attribut sous la forme d'un Tableau dynamique de trajets.
// On parcours ensuite ce tableau en recherchant les trajets existant pours lesquels les villes de départ
// et d'arrivée correspondent à celles entrées par l'utilisateur. Si ces trajets existent, on appelle leur méthode
// Affichage
//
{
    char * vDepart=askVilleDepart();
    char * vArrivee=askVilleArrivee();
    bool trouve = false;
    int cpt = 0;

    cout << endl << "-- Résultat de la recherche simple entre " << vDepart << " et " << vArrivee << " --" << endl << endl;

    Trajet ** listeTmp = listeTraj.GetListe();
    int nbTrajets = listeTraj.GetNbTrajets();
    for(int i=0;i<nbTrajets;i++)
    {
        if(strcmp(listeTmp[i]->GetVilleDepart(),vDepart)==0 && strcmp(listeTmp[i]->GetVilleArrivee(),vArrivee)==0)
        {
            listeTmp[i]->Affichage();
            trouve=true;
            ++cpt;
            cout << endl;
        }
    }

    if(!trouve)
    {
        cout << "            AUCUN TRAJET DISPONIBLE" << endl << endl;
    }

    cout << "           Nombre de trajets trouvés : " << cpt << endl;
    cout << "-----------------------------------------------------------" << endl;

    delete [] vDepart;
    delete [] vArrivee;
} //----- Fin de RechercheSimple


void Catalogue::RechercheComplexe()
// Algorithme :
// On crée une liste de trajet vide et on demande à l'utilisateur de saisir la ville de départ initale et la destination finale.
// On appelle ensuite la méthode rechercheEtape avec ces trois paramètres.
// La méthode rechercheEtape retourne alors le nombre de trajets trouvés, que l'on affiche. L'affichage de ces trajets est gérée par RechercheEtape
{
    char * vDepart=askVilleDepart();
    char * vArrivee=askVilleArrivee();
    ListeTrajets trajetsPossibles = ListeTrajets();

    cout << endl << "-- Résultat de la recherche complexe entre " << vDepart << " et " << vArrivee << " --" << endl << endl ;

    int nb = rechercheEtape(vDepart,vArrivee,trajetsPossibles);

    if(nb==0)
    {
        cout << "            AUCUN TRAJET DISPONIBLE" << endl;
    }

    cout << "---- Fin de la recherche complexe : " << nb << " ont étés proposées ----" << endl;

    delete [] vDepart;
    delete [] vArrivee;
} //----- Fin de RechercheComplexe

//------------------------------------------------- Surcharge d'opérateurs

ostream & operator << (ostream & flux, const Catalogue & unCatalogue)
// Algorithme :
// Parcours l'attribut ListeTraj pou afficher les trajets présents dans le catalogue.
// Pour simplifier le parcours, on vient récupérer la liste de trajets sous la forme d'un
// tableau dynamique.
{
    int nbTrajets = unCatalogue.listeTraj.GetNbTrajets();
    cout << nbTrajets << endl;
    if(nbTrajets==0)
    {
        cout << "---------- CATALOGUE VIDE ----------" << endl;
    }
    else
    {
        Trajet ** traj = unCatalogue.listeTraj.GetListe();
        cout << "---------- CONTENU DU CATALOGUE ----------" << endl;
        cout << endl;
        for(int i=0; i<nbTrajets;i++)
        {
            traj[i]->Affichage();
            cout << endl;
        }

        cout << "-------------------------------------------" << endl;
    }
} //----- Fin de surcharge opérateur <<


//-------------------------------------------- Constructeurs - destructeur

Catalogue::Catalogue () : listeTraj()
// Algorithme :
//
{
#ifdef MAP
    cout << "Appel au constructeur de <Catalogue>" << endl;
#endif
} //----- Fin de Catalogue


Catalogue::~Catalogue ( )
// Algorithme :
// On parcours toute la liste listeTraj afin de détruire chacun des trajets présents.
{
#ifdef MAP
    cout << "Appel au destructeur de <Catalogue>" << endl;
#endif
    for(int i=0;i<listeTraj.GetNbTrajets();i++)
    {
        delete listeTraj.GetListe()[i];
    }
} //----- Fin de ~Catalogue


//------------------------------------------------------------------ PRIVE

//----------------------------------------------------- Méthodes protégées

bool Catalogue::respectCriteria(Trajet * trajet, CritereSelection critere, string param1, string param2, int param3)
// Algorithme
// Lecture via le pointeur de trajets si les attributs du trajet correspondent au critère demandé.
// L'emploi d'un string permet l'utilisation des '==' pour la comparaison.
// 
{

    bool useable = false;

    string choix;
    string vDepart;
    string vArrivee;
    int numero(0);
    int intervalStart(0);
    int intervalEnd(0);

    switch(critere)
    {
        case SANS : //Trajet respecte toujours le critère s'il n'existe pas

            useable = true;
            break;

        case TYPE : //Trajet doit être du bon type

            choix = param1;

            if(trajet->GetType()==choix)
            {
                useable = true;
            }

            break;

        case VILLE : // Trajet doit respecter la ville de départ et/ou d'arrivée
            // Si il n'y a ni ville de départ ni d'arrivée, revient à aucun critère

            vDepart = param1;
            vArrivee = param2;

            useable = true;

            if(!vDepart.empty() && vDepart!=trajet->GetVilleDepart())
            {
                useable=false;
            }

            if(!vArrivee.empty() && vArrivee!=trajet->GetVilleArrivee())
            {
                useable=false;
            }

            break;

        case TRAJETS : //Trajet bon si son numéro est bien dans l'intervalle demandé

            intervalStart = stoi(param1);
            intervalEnd = stoi(param2);
            numero = param3;

            if(numero >= intervalStart && numero <= intervalEnd)
            {
                useable = true;
            }

            break;

        default:
            cerr << "Une erreur s'est produite dans l'appel d'un critère de sélection" << endl;
            useable = false;
            break;
    }

    return useable;
} // ------- Fin de respectCriteria

void Catalogue::askParameters(CritereSelection critere, string & param1, string & param2)
//Algorithme
//  Respect des critères :
//      Critère SANS : param1 --> / - param2 --> /
//      Critère TYPE : param1 --> TS ou TC - param2 --> /
//      Critère VILLE : param1 --> vDepart - param2 --> vArrivee
//      Critère TRAJETS cad entre [n,m] : param1 --> n - param2 --> m
// Interfaçage avec l'utilisateur en fonction du critère demandé.
// Emploi d'une conversion string -> int et int -> string via stoi et to_string pour le parametre 2 lors du choix
// de l'intervalle.
{


    string choix("");
    switch(critere) //Entrées des paramètres
    {
        case SANS :
            break;

        case TYPE :
            cout << "Sélection du type de trajets souhaité lors de la sauvegarde\n\t* TS\n\t* TC" << endl;
            cin >> param1;

            while(cin.fail() || !(param1 == "TC" || param1 == "tc" || param1 == "TS" || param1 == "ts"))
            {
                cin.clear();
                cin.ignore(10000,'\n');

                cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                cin >> param1;
            }

            break;

        case VILLE :
            cout << "Souhaitez vous sélectionner une ville de départ ? (Y/N)" << endl;
            cin >> choix;

            while(cin.fail() || !(choix == "Y" || choix == "y" || choix == "N" || choix == "n"))
            {
                cin.clear();
                cin.ignore(10000,'\n');

                cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                cin >> choix;
            }

            if(choix=="Y" || choix == "y")
            {
                cout << "Saisir la ville de départ :" << endl;
                cin >> param1;

                while(cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000,'\n');

                    cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                    cin >> param1;
                }
            }

            cout << "Souhaitez vous sélectionner une ville d'arrivée ? (Y/N)" << endl;
            cin >> choix;

            while(cin.fail() || !(choix == "Y" || choix == "y" || choix == "N" || choix == "n"))
            {
                cin.clear();
                cin.ignore(10000,'\n');

                cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                cin >> choix;
            }

            if(choix=="Y" || choix == "y")
            {
                cout << "Saisir la ville d'arrivée :" << endl;
                cin >> param2;

                while(cin.fail())
                {
                    cin.clear();
                    cin.ignore(10000,'\n');

                    cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                    cin >> param2;
                }

            }
            break;

        case TRAJETS :
            cout << "Saisir la borne inférieure n de l'intervalle [n,m] avec n>=1 : " << endl;

            int tmpForErrorsInf;
            cin >> tmpForErrorsInf;

            while(cin.fail() || tmpForErrorsInf<1)
            {
                cin.clear();
                cin.ignore(10000,'\n');

                cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                cin >> tmpForErrorsInf;
            }

            param1=to_string(tmpForErrorsInf);

            cout << "Saisir la borne supérieure m de l'intervalle [n,m] avec m>=n : " << endl;

            int tmpForErrorsSup;
            cin >> tmpForErrorsSup;

            while(cin.fail() || tmpForErrorsSup<tmpForErrorsInf)
            {
                cin.clear();
                cin.ignore(10000,'\n');

                cout << "Mauvaise saisie. Veuillez recommencer." << endl;
                cin >> tmpForErrorsSup;
            }

            param2=to_string(tmpForErrorsSup);

            break;

        default:
            cerr << "Une erreur s'est produite dans l'appel d'un critère de sélection" << endl;
            break;
    }
} // ----- Fin de askParameters

int Catalogue::putInFile(ostream &stream, int &counter, ostringstream &buffer)
// Algorithme
// Si le stream est bien ouvert, écrit l'intégralité du buffer à l'interieur et augmenter le compteur.
{
    if (!buffer.str().empty()) {
        if (stream) {
            stream << buffer.str();
            counter++;
        } else {
            cerr << "Une erreur s'est produite lors de l'écriture. Abandon" << endl;
            return -1;
        }
    }

    return 0;
} // ----- Fin de putInFile

ostringstream & Catalogue::createBuffer(ostringstream &buffer, Trajet *traj)
// Algorithme :
// Pour le trajet passé en paramètre, récupère ses informations via les getters et en compose un string
// que l'on pourra écrire dans le fichier de sortie tel quel.
// Il est important d'y ajouter un retour à la ligne pour que chaque ligne correspondante à un trajet unique.
{
    buffer.str("");
    buffer.clear();

    buffer<<(traj->GetType()+"|"+traj->GetVilleDepart()+"|"+traj->GetVilleArrivee()+"|");

    if(traj->GetType()=="TS")
    {
        TrajetSimple* ts = dynamic_cast<TrajetSimple*>(traj);
        if(ts == nullptr)
        {
            cerr << "Erreur dynamic_cast lors de la sauvegarde" << endl;
            buffer.str("");
            buffer.clear();
            return buffer;
        }
        buffer<<ts->GetMoyenTransport();
    }
    else //TC
    {
        TrajetCompose* tc = dynamic_cast<TrajetCompose*>(traj);
        if(tc == nullptr)
        {
            cerr << "Erreur dynamic_cast lors de la sauvegarde" << endl;
            buffer.str("");
            buffer.clear();
            return buffer;
        }

        int nbTrajets = tc->GetNbTrajets();

        buffer<<to_string(nbTrajets)+"|";

        ListeTrajets * etapes = tc->GetEtapes();

        for(int i = 0; i<(etapes->GetNbTrajets()-1);i++)
        {
            TrajetSimple * tmp = dynamic_cast<TrajetSimple*> (etapes->GetListe()[i]);

            if(tmp == nullptr)
            {
                cerr << "Erreur dynamic_cast lors de la sauvegarde" << endl;
                buffer.str("");
                buffer.clear();
                return buffer;
            }

            buffer<<tmp->GetVilleArrivee();
            buffer<<"|";
            buffer<<tmp->GetMoyenTransport();
            buffer<<"|";
        }

        TrajetSimple * tmp = dynamic_cast<TrajetSimple*>(etapes->GetListe()[etapes->GetNbTrajets()-1]);

        if(tmp == nullptr)
        {
            cerr << "Erreur dynamic_cast lors de la sauvegarde" << endl;
            buffer.str("");
            buffer.clear();
            return buffer;
        }

        buffer<<tmp->GetMoyenTransport();

    }

    buffer<<"\n";

    return buffer;
} // ------ Fin de createBuffer

Trajet* Catalogue::construitTrajetAvecLecture(ifstream & fichier)
// Algorithme :
// Cette méthode va lire une ligne entière du fichier passé en paramètre et construire un trajet à partir de cette ligne.
// La méthode s'adapte au cas où la ligne correspond à un trajet simple ou composé et lit une à une les informations concernant le trajet en cours de construction.
//
// Au final elle retourne un pointeur de type Trajet, qui pointe vers le trajet qui a été créé.
{
    char typeTrajet[15];
    char villeDepart[TAILLE_ENTREE_VILLE];
    char villeArrivee[TAILLE_ENTREE_VILLE];
    char villeDepartIniTC[TAILLE_ENTREE_VILLE];
    char villeArriveeFinTC[TAILLE_ENTREE_VILLE];
    char moyenTransport[TAILLE_ENTREE_MOYEN_TRANSPORT];
    char lectureNbTrajet[5];
    int nbTrajetDansTC;
    Trajet * ptr_trajet;
    int i;

    //lecture du type
    fichier.getline(typeTrajet,10,'|');
    if(!fichier) // on a atteint la fin du fichier
    {
        return nullptr;
    }
    if(strcmp(typeTrajet,"TS")==0) // trajet simple
    {
        fichier.getline(villeDepart,TAILLE_ENTREE_VILLE,'|');
        fichier.getline(villeArrivee,TAILLE_ENTREE_VILLE,'|');
        fichier.getline(moyenTransport,TAILLE_ENTREE_MOYEN_TRANSPORT);
        ptr_trajet = new TrajetSimple(villeDepart,villeArrivee,moyenTransport);
        return ptr_trajet;
    }
    else// trajet compose
    {
        fichier.getline(villeDepartIniTC,TAILLE_ENTREE_VILLE,'|');
        fichier.getline(villeArriveeFinTC,TAILLE_ENTREE_VILLE,'|');
        fichier.getline(lectureNbTrajet,5,'|');
        //on convertit le nb trajet lu en int
        stringstream s(lectureNbTrajet);
        s>>nbTrajetDansTC;
        ListeTrajets * listeTrajetTC = new ListeTrajets(nbTrajetDansTC);

        for(i=0;i<nbTrajetDansTC;i++)
        {
            if(i==0)
            {
                strcpy(villeDepart,villeDepartIniTC);
            }
            else
            {
                strcpy(villeDepart,villeArrivee);
            }

            if(i==nbTrajetDansTC-1) // cas particulier car pour le dernier trajet on a juste besoin de relever le moyen de transport (et il n'y a pas de | à la fin)
            {
                strcpy(villeArrivee,villeArriveeFinTC);
                fichier.getline(moyenTransport,TAILLE_ENTREE_MOYEN_TRANSPORT);
            }
            else
            {
                fichier.getline(villeArrivee,TAILLE_ENTREE_VILLE,'|');
                fichier.getline(moyenTransport,TAILLE_ENTREE_MOYEN_TRANSPORT,'|');
            }

            ptr_trajet = new TrajetSimple(villeDepart,villeArrivee,moyenTransport);
            listeTrajetTC->AddTrajet(ptr_trajet);
        }
        ptr_trajet = new TrajetCompose(villeDepartIniTC,villeArriveeFinTC,listeTrajetTC);
        return ptr_trajet;

    }

} //----- Fin de construitTrajetAvecLecture

char * Catalogue::askVilleArrivee()
// Algorithme :
//
{
    char* ret=new char[TAILLE_ENTREE_VILLE+1];
    cout << "Saisir la ville d'arrivée: " << endl;
    cin.getline(ret,TAILLE_ENTREE_VILLE);
    return ret;
} //----- Fin de AskVilleDepart

char * Catalogue::askVilleDepart()
// Algorithme :
//
{
    char* ret = new char[TAILLE_ENTREE_VILLE+1];
    cout << "Saisir la ville de départ : " << endl;
    cin.getline(ret,TAILLE_ENTREE_VILLE);
    return ret;
} //----- Fin de AskVilleArrivee

char * Catalogue::askMoyenTransport()
// Algorithme :
//
{
    char* ret=new char[TAILLE_ENTREE_MOYEN_TRANSPORT+1];
    cout << "Saisir le moyen de transport : " << endl;
    cin.getline(ret,TAILLE_ENTREE_VILLE);
    return ret;
} //----- Fin de AskMoyenTransport

int Catalogue::rechercheEtape(const char * departTrajet, const char * arriveeFinale, ListeTrajets trajetsPossibles)
//Algorithme : Methode recursive
// departTrajet représente la ville actuellement étudiée, arrivéeFinale représente la ville de destination finale, et trajetsPossibles contient tous les trajets qui ont déjà été parcourus
// permettant d'arriver à la ville actuellement étudiée (departTrajet).
// Cette méthode est récursive. On parcours la liste des trajets présent dans Catalogue, et pour chacun de ces trajets T, si la ville de départ correspond à départTrajet, on réapelle cette méthode en
// Ajoutant le trajet T à la liste trajetsPossibles, et en passant la ville d'arrivée du trajet T dans départTrajet. cette ville devient alors la prochaine étape etudiée.
//
// Si la ville actuellement étudiée est la même que l'arriveeFinale: On a notre condition d'arrêt. Dans ce cas la liste trajetsPossibles contient une liste de trajets permettant de relier la ville
// Initiale à la destination Finale: On affiche alors ces trajets et on retourne 1 (le retour représente le nombre de trajets différents trouvés)
// Soit on a parcouru toute la liste de Trajets listeTraj sans trouver de nouveau trajet partant de departTrajet. Dans ce cas on
{
    int condition;
    int compteurTrajets = 0;

    if(strcmp(departTrajet,arriveeFinale)==0) // si ville de départ et d'arrivee sont les mêmes: signifie qu'on à atteint notre destination
    {

        for(int i = 0;i<trajetsPossibles.GetNbTrajets();i++)
        {
            trajetsPossibles.GetListe()[i]->Affichage();
        }

        cout << endl;

        return 1;
    }

    for(int i=0;i<listeTraj.GetNbTrajets();i++)
    {
        Trajet* t=listeTraj.GetListe()[i];

        if(strcmp(t->GetVilleDepart(),departTrajet)==0) //si le trajet t auquel on s'intéresse part bien du même point que le noeud ou l'on est actuellement
        {
            condition=1;

            for(int j=0;j<trajetsPossibles.GetNbTrajets();j++)
            {
                if (strcmp(t->GetVilleArrivee(), trajetsPossibles.GetListe()[j]->GetVilleDepart()) ==0) //si la ville d'arrivée est déja présente dans les trajets possible en tant que ville de départ
                {
                    condition = 0;        //afin de ne pas créer de boucle infini
                }
            }
            if(condition)
            {
                trajetsPossibles.AddTrajet(t);                                       //On ajoute une arrête à explorer à partir de ce noeud
                compteurTrajets+=rechercheEtape(t->GetVilleArrivee(),arriveeFinale,trajetsPossibles);  //On part explorer cette arrête
                trajetsPossibles.RetirerDernier();                                   //On retire cette arrête pour ne pas l'explorer à partir des autres noeuds
            }
        }
    }

    return compteurTrajets;
}// Fin de rechercheEtape
