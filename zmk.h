#ifndef ZOS_ZMK_H
#define ZOS_ZMK_H


/*
 * File:   main.cpp
 * Author: zmk
 *
 * Created on 7. prosince 2016, 13:27
 */


#include <string.h>
#include <iostream>
#include <cstdint>
#include <cstdbool>
#include <stdio.h>
#include <stdint.h>

//pocitame s FAT32 MAX - tedy horni 4 hodnoty
const int32_t FAT_UNUSED = INT32_MAX - 1;
const int32_t FAT_FILE_END = INT32_MAX - 2;
const int32_t FAT_BAD_CLUSTER = INT32_MAX - 3;
const int32_t FAT_DIRECTORY = INT32_MAX - 4;

struct boot_record {
    char volume_descriptor[250];    //popis vygenerovaného FS
    int8_t fat_type;                //typ FAT (FAT12, FAT16...) 2 na fat_type - 1 clusterů
    int8_t fat_copies;              //počet kopií FAT tabulek
    int16_t cluster_size;           //velikost clusteru
    int32_t usable_cluster_count;   //max počet clusterů, který lze použít pro data (-konstanty)
    char signature[9];              //login autora FS
};// 272B

//pokud bude ve FAT FAT_DIRECTORY, budou na disku v daném clusteru uloženy struktury o velikosti sizeof(directory) = 24B
struct directory{
    char name[13];                  //jméno souboru, nebo adresáře ve tvaru 8.3'/0' 12 + 1
    bool isFile;                    //identifikace zda je soubor (TRUE), nebo adresář (FALSE)
    int32_t size;                   //velikost položky, u adresáře 0
    int32_t start_cluster;          //počáteční cluster položky
};// 24B


/*
using namespace std;

int main(int argc, char** argv) {

    FILE *fp;
    struct boot_record br;
    struct directory root_a, root_b, root_c, root_d;

    //boot_rec
    //smazani vsech B na /0 u stringu
    memset(br.signature, '\0', sizeof(br.signature));
    memset(br.volume_descriptor, '\0', sizeof(br.volume_descriptor));
    strcpy(br.volume_descriptor, "Testiovaci - FAT8 - tedy 256 clusteru, cluster 256B - 4 rezervovane - pouzitelne 0-251 (zde neni nutne rezervovat)");
    br.fat_type = 8;
    br.fat_copies = 2;
    br.cluster_size = 256;
    br.usable_cluster_count = 251;
    strcpy(br.signature, "zmk");

    //directory - vytvoreni polozek
    memset(root_a.name, '\0', sizeof(root_a.name));
    root_a.isFile = 1;
    strcpy(root_a.name,"cisla.txt");
    root_a.size = 135;
    root_a.start_cluster = 1;

    memset(root_b.name, '\0', sizeof(root_b.name));
    root_b.isFile = 1;
    strcpy(root_b.name,"pohadka.txt");
    root_b.size = 5975;
    root_b.start_cluster = 2;

    memset(root_c.name, '\0', sizeof(root_c.name));
    root_c.isFile = 1;
    strcpy(root_c.name,"msg.txt");
    root_c.size = 396;
    root_c.start_cluster = 30;

    memset(root_d.name, '\0', sizeof(root_d.name));
    root_d.isFile = 0;
    strcpy(root_d.name,"direct-1");
    root_d.size = 0;
    root_d.start_cluster = 29;

    // pro zapis budu potrebovat i prazdny cluster
    char cluster_empty[br.cluster_size];
    ////////////////////////////////////////////// SOUBORY A ADRESARE POKUSNE
    char cluster_dir1[br.cluster_size];
    char cluster_a[br.cluster_size];
    char cluster_b1[br.cluster_size];
    char cluster_b2[br.cluster_size];
    char cluster_b3[br.cluster_size];
    char cluster_b4[br.cluster_size];
    char cluster_b5[br.cluster_size];
    char cluster_b6[br.cluster_size];
    char cluster_b7[br.cluster_size];
    char cluster_b8[br.cluster_size];
    char cluster_b9[br.cluster_size];
    char cluster_b10[br.cluster_size];
    char cluster_b11[br.cluster_size];
    char cluster_b12[br.cluster_size];
    char cluster_b13[br.cluster_size];
    char cluster_b14[br.cluster_size];
    char cluster_b15[br.cluster_size];
    char cluster_b16[br.cluster_size];
    char cluster_b17[br.cluster_size];
    char cluster_b18[br.cluster_size];
    char cluster_b19[br.cluster_size];
    char cluster_b20[br.cluster_size];
    char cluster_b21[br.cluster_size];
    char cluster_b22[br.cluster_size];
    char cluster_b23[br.cluster_size];
    char cluster_b24[br.cluster_size];
    char cluster_c1[br.cluster_size];
    char cluster_c2[br.cluster_size];

    //pripravim si obsah - delka stringu musi byt stejna jako velikost clusteru
    memset(cluster_empty, '\0', sizeof(cluster_empty));
    memset(cluster_a, '\0', sizeof(cluster_a));
    memset(cluster_b1, '\0', sizeof(cluster_b1));
    memset(cluster_b2, '\0', sizeof(cluster_b2));
    memset(cluster_b3, '\0', sizeof(cluster_b3));
    memset(cluster_b4, '\0', sizeof(cluster_b4));
    memset(cluster_b5, '\0', sizeof(cluster_b5));
    memset(cluster_b6, '\0', sizeof(cluster_b6));
    memset(cluster_b7, '\0', sizeof(cluster_b7));
    memset(cluster_b8, '\0', sizeof(cluster_b8));
    memset(cluster_b9, '\0', sizeof(cluster_b9));
    memset(cluster_b10, '\0', sizeof(cluster_b10));
    memset(cluster_b11, '\0', sizeof(cluster_b11));
    memset(cluster_b12, '\0', sizeof(cluster_b12));
    memset(cluster_b13, '\0', sizeof(cluster_b13));
    memset(cluster_b14, '\0', sizeof(cluster_b14));
    memset(cluster_b15, '\0', sizeof(cluster_b15));
    memset(cluster_b16, '\0', sizeof(cluster_b16));
    memset(cluster_b17, '\0', sizeof(cluster_b17));
    memset(cluster_b18, '\0', sizeof(cluster_b18));
    memset(cluster_b19, '\0', sizeof(cluster_b19));
    memset(cluster_b20, '\0', sizeof(cluster_b20));
    memset(cluster_b21, '\0', sizeof(cluster_b21));
    memset(cluster_b22, '\0', sizeof(cluster_b22));
    memset(cluster_b23, '\0', sizeof(cluster_b23));
    memset(cluster_b24, '\0', sizeof(cluster_b24));
    memset(cluster_c1, '\0', sizeof(cluster_c1));
    memset(cluster_c2, '\0', sizeof(cluster_c2));
    memset(cluster_dir1, '\0', sizeof(cluster_dir1));
strcpy(cluster_a,"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789 - tohle je malicky soubor pro test");
    strcpy(cluster_b1, "Byla jednou jedna sladka divenka, kterou musel milovat kazdy, jen ji uvidel, ale nejvice ji milovala jeji babicka, ktera by ji snesla i modre z nebe. Jednou ji darovala cepecek karkulku z cerveneho sametu a ten se vnucce tak libil, ze nic jineho nechtela ");
    strcpy(cluster_b2, "nosit, a tak ji zacali rikat Cervena Karkulka. Jednou matka Cervene Karkulce rekla: „Podivej, Karkulko, tady mas kousek kolace a lahev vina, zanes to babicce, je nemocna a zeslabla, timhle se posilni. Vydej se na cestu drive nez bude horko, jdi hezky spor");
    strcpy(cluster_b3, "adane a neodbihej z cesty, kdyz upadnes, lahev rozbijes a babicka nebude mit nic. A jak vejdes do svetnice, nezapomeň babicce poprat dobreho dne a ne abys smejdila po vsech koutech.“ „Ano, maminko, udelam, jak si prejete.“ rekla Cerveni Karkulka, na stvrz");
    strcpy(cluster_b4, "eni toho slibu podala matce ruku a vydala se na cestu. Babicka bydlela v lese; celou půlhodinu cesty od vesnice. Kdyz sla Cervena Karkulka lesem, potkala vlka. Tenkrat jeste nevedela, co je to za zaludne zvire a ani trochu se ho nebala. „Dobry den, Cerven");
    strcpy(cluster_b5, "a Karkulko!“ rekl vlk. „Dekuji za prani, vlku.“ „Kampak tak casne, Cervena Karkulko?“ „K babicce!“ „A copak to neses v zasterce?“ „Kolac a vino; vcera jsme pekli, nemocne a zeslable babicce na posilnenou.“ „Kdepak bydli babicka, Cervena Karkulko?“ „Inu, j");
    strcpy(cluster_b6, "este tak ctvrthodiny cesty v lese, jeji chaloupka stoji mezi tremi velkymi duby, kolem je liskove oresi, urcite to tam musis znat.“ odvetila Cervena Karkulka. Vlk si pomyslil: „Tohle mlaďoucke, jemňoucke masicko bude jiste chutnat lepe nez ta starena, mus");
    strcpy(cluster_b7, "im to navleci lstive, abych schlamstnul obe.“ Chvili sel vedle Cervene Karkulky a pak pravil: „Cervena Karkulko, koukej na ty krasne kvetiny, ktere tu rostou vsude kolem, procpak se trochu nerozhlednes? Myslim, ze jsi jeste neslysela ptacky, kteri by zpiv");
    strcpy(cluster_b8, "ali tak libezne. Ty jsi tu vykracujes, jako kdybys sla do skoly a pritom je tu v lese tak krasne!“ Cervena Karkulka otevrela oci dokoran a kdyz videla, jak slunecni paprsky tancuji skrze stromy sem a tam a vsude roste tolik krasnych kvetin, pomyslila si: ");
    strcpy(cluster_b9, "„Kdyz prinesu babicce kytici cerstvych kvetin, bude mit jiste radost, casu mam dost, prijdu akorat.“ A sebehla z cesty do lesa a trhala kvetiny. A kdyz jednu utrhla, zjistila, ze o kus dal roste jeste krasnejsi, bezela k ni, a tak se dostavala stale hloub");
    strcpy(cluster_b10,"eji do lesa. Ale vlk bezel rovnou k babiccine chaloupce a zaklepal na dvere. „Kdo je tam?“ „Cervena Karkulka, co nese kolac a vino, otevri!“ „Jen zmackni kliku,“ zavolala babicka: „jsem prilis slaba a nemohu vstat.“ Vlk vzal za kliku, otevrel dvere a beze");
    strcpy(cluster_b11,"slova sel rovnou k babicce a spolknul ji. Pak si obleknul jeji saty a nasadil jeji cepec, polozil se do postele a zatahnul zaves. Zatim Cervena Karkulka behala mezi kvetinami, a kdyz jich mela naruc tak plnou, ze jich vic nemohla pobrat, tu ji prisla na  ");
    strcpy(cluster_b12,"mysl babicka, a tak se vydala na cestu za ni. Podivila se, ze jsou dvere otevrene, a kdyz vesla do svetnice, prislo ji vse takove podivne, ze si pomyslila: „Dobrotivy Boze, je mi dneska nejak úzko a jindy jsem u babicky tak rada.“ Zvolala: „Dobre jitro!“ ");
    strcpy(cluster_b13,"Ale nedostala zadnou odpoveď. Šla tedy k posteli a odtahla zaves; lezela tam babicka a mela cepec narazeny hluboko do obliceje a vypadala nejak podivne. Ach, babicko, proc mas tak velke usi?“ „Abych te lepe slysela.“ „Ach, babicko, proc mas tak velke oci ");
    strcpy(cluster_b14,"?“ „Abych te lepe videla.“ „Ach, babicko, proc mas tak velke ruce?“ „Abych te lepe objala.“ „Ach, babicko, proc mas tak straslivou tlamu?“ „Abych te lepe sezrala!!“ Sotva vlk ta slova vyrknul, vyskocil z postele a ubohou Cervenou Karkulku spolknul. Kdyz t");
    strcpy(cluster_b15,"eď uhasil svoji zadostivost, polozil se zpatky do postele a usnul a z toho spanku se jal mocne chrapat. Zrovna sel kolem chaloupky lovec a pomyslil si: „Ta starenka ale chrape, musim se na ni podivat, zda neco nepotrebuje.“ Vesel do svetnice, a kdyz prist");
    strcpy(cluster_b16,"oupil k posteli, uvidel, ze v ni lezi vlk. „Tak prece jsem te nasel, ty stary hrisniku!“ zvolal lovec: „Uz mam na tebe dlouho policeno!“ Strhnul z ramene pusku, ale pak mu prislo na mysl, ze vlk mohl sezrat babicku a mohl by ji jeste zachranit. Nestrelil ");
    strcpy(cluster_b17,"tedy, nybrz vzal nůzky a zacal spicimu vlkovi parat bricho. Sotva ucinil par rezů, uvidel se cervenat karkulku a po par dalsich rezech vyskocila divenka ven a volala: „Ach, ja jsem se tolik bala, ve vlkovi je cernocerna tma.“ A potom vylez la ven i ziva b");
    strcpy(cluster_b18,"abicka; sotva dechu popadala. Cervena Karkulka pak nanosila obrovske kameny, kterymi vlkovo bricho naplnili, a kdyz se ten probudil a chtel uteci, kameny ho tak desive tizily, ze klesnul k zemi nadobro mrtvy. Ti tri byli spokojeni. Lovec stahnul vlkovi ko");
    strcpy(cluster_b19,"zesinu a odnesl si ji domů, babicka snedla kolac a vypila vino, ktere Cervena Karkulka prinesla, a opet se zotavila. A Cervena Karkulka? Ta si svatosvate prisahala: „Uz nikdy v zivote nesejdu z cesty do lesa, kdyz mi to maminka zakaze!“ O Cervene  Karkulc");
    strcpy(cluster_b20,"e se jeste vypravi, ze kdyz sla jednou zase k babicce s babovkou, potkala jineho vlka a ten se ji taky vemlouval a snazil se ji svest z cesty. Ale  ona se toho vystrihala a kracela rovnou k babicce, kde hned vypovedela, ze potkala vlka, ktery ji sice popr");
    strcpy(cluster_b21,"al dobry den, ale z oci mu koukala nekalota. „Kdyby to nebylo na verejne ceste, jiste by mne sezral!“ „Pojď,“  rekla babicka: „zavreme dobre dvere, aby nemohl dovnitr.“ Brzy nato zaklepal vlk a zavolal: „Otevri, babicko, ja jsem Cervena Karkulka a nesu ti");
    strcpy(cluster_b22,"pecivo!“ Ty dve vsak zůstaly jako peny a neotevrely. Tak se ten sedivak plizil kolem domu a naslouchal, pak vylezl na strechu, aby tam pockal, az Cervena Karkulka půjde vecer domů, pak ji v temnote popadne a sezere. Ale babicka zle vlkovy úmysly odhalila ");
    strcpy(cluster_b23,". Pred domem staly obrovske kamenne necky, tak Cervene  Karkulce rekla: „Vezmi vedro, devenko, vcera jsem varila klobasy, tak tu vodu nanosime venku do necek.“ Kdyz byly necky plne, stoupala vůne klobas nahoru az k vlkovu cenichu. Zavetril a natahoval krk");
    strcpy(cluster_b24,"tak daleko, ze se na strese vice neudrzel a zacal klouzat dolů, kde spadnul primo do necek a bidne se utopil.");
    strcpy(cluster_c1, "Prodej aktiv SABMilleru v Ceske republice, Polsku, Maďarsku, Rumunsku a na Slovensku je soucasti podminek pro prevzeti podniku ze strany americkeho pivovaru Anheuser-Busch InBev, ktere bylo dokonceno v rijnu. Krome Plzeňskeho Prazdroje zahrnuji prodavana ");
    strcpy(cluster_c2, "aktiva polske znacky Tyskie a Lech, slovensky Topvar, maďarsky Dreher a rumunsky Ursus. - Tento soubor je sice kratky, ale neni fragmentovany");

/////////// ZACATEK VYTVARENI FAT TABULKY

    int32_t fat[br.usable_cluster_count];

    //ve fatce na 0 clusteru bude root directory
    fat[0] = FAT_DIRECTORY;
    //pak bude soubor "cisla.txt" - ten je jen jednoclusterovy
    fat[1] = FAT_FILE_END;
    //pak bude dlouhy soubor "pohadka.txt", ktery je cely za sebou
    fat[2] = 3;
    fat[3] = 4;
    fat[4] = 5;
    fat[5] = 6;
    fat[6] = 7;
    fat[7] = 8;
    fat[8] = 9;
    fat[9] = 10;
    fat[10] = 11;
    fat[11] = 12;
    fat[12] = 13;
    fat[13] = 14;
    fat[14] = 15;
    fat[15] = 16;
    fat[16] = 17;
    fat[17] = 18;
    fat[18] = 19;
    fat[19] = 20;
    fat[20] = 21;
    fat[21] = 22;
    fat[22] = 23;
    fat[23] = 24;
    fat[24] = 25;
    fat[25] = FAT_FILE_END;
    //ted bude nejake volne misto
    fat[26] = FAT_UNUSED;
    fat[27] = FAT_UNUSED;
    fat[28] = FAT_UNUSED;
    //pak adresar "directory-1"
    fat[29] = FAT_DIRECTORY;
    //pak soubor "msg.txt"
    fat[30] = 33;
    //bohuzel pri jeho zapisu se se muselo fragmenotvat - jsou tu dva spatne sektory
    fat[31] = FAT_BAD_CLUSTER;
    fat[32] = FAT_BAD_CLUSTER;
    //a tady je konec "msg.txt"
    fat[33] = FAT_FILE_END;
    //zbytek bude prazdny
    for (int32_t i = 34; i < br.usable_cluster_count; i++)
    {
        fat[i] = FAT_UNUSED;
    }

/////////// KONEC VYTVARENI FAT TABULKY


    fp = fopen("empty.fat", "w");
    //boot record
        fwrite(&br, sizeof(br), 1, fp);
    // 2x FAT
        fwrite(&fat, sizeof(fat), 1, fp);
        fwrite(&fat, sizeof(fat), 1, fp);
    //dir - bacha, tady je potreba zapsat CELY CLUSTER, ne jen prvnich n-BYTU plozek - tedy doplnit nulami poradi zaznamu v directory NEMUSI odpovidat poradi ve FATce a na disku
    int16_t cl_size = br.cluster_size;
    int16_t ac_size = 0;
        fwrite(&root_a, sizeof(root_a), 1, fp);
        ac_size += sizeof(root_a);
        fwrite(&root_b, sizeof(root_b), 1, fp);
        ac_size += sizeof(root_b);
        fwrite(&root_c, sizeof(root_c), 1, fp);
        ac_size += sizeof(root_c);
        fwrite(&root_d, sizeof(root_d), 1, fp);
        ac_size += sizeof(root_d);
        char buffer[] = { '\0' };
        for (int16_t i = 0; i < (cl_size - ac_size); i++)
            fwrite(buffer, sizeof(buffer), 1, fp);
    //data - soubor cisla.txt
     fwrite(&cluster_a, sizeof(cluster_a), 1, fp);
     //data - soubor pohadka.txt
     fwrite(&cluster_b1, sizeof(cluster_b1), 1, fp);
     fwrite(&cluster_b2, sizeof(cluster_b2), 1, fp);
     fwrite(&cluster_b3, sizeof(cluster_b3), 1, fp);
     fwrite(&cluster_b4, sizeof(cluster_b4), 1, fp);
     fwrite(&cluster_b5, sizeof(cluster_b5), 1, fp);
     fwrite(&cluster_b6, sizeof(cluster_b6), 1, fp);
     fwrite(&cluster_b7, sizeof(cluster_b7), 1, fp);
     fwrite(&cluster_b8, sizeof(cluster_b8), 1, fp);
     fwrite(&cluster_b9, sizeof(cluster_b9), 1, fp);
     fwrite(&cluster_b10, sizeof(cluster_b10), 1, fp);
     fwrite(&cluster_b11, sizeof(cluster_b11), 1, fp);
     fwrite(&cluster_b12, sizeof(cluster_b12), 1, fp);
     fwrite(&cluster_b13, sizeof(cluster_b13), 1, fp);
     fwrite(&cluster_b14, sizeof(cluster_b14), 1, fp);
     fwrite(&cluster_b15, sizeof(cluster_b15), 1, fp);
     fwrite(&cluster_b16, sizeof(cluster_b16), 1, fp);
     fwrite(&cluster_b17, sizeof(cluster_b17), 1, fp);
     fwrite(&cluster_b18, sizeof(cluster_b18), 1, fp);
     fwrite(&cluster_b19, sizeof(cluster_b19), 1, fp);
     fwrite(&cluster_b20, sizeof(cluster_b20), 1, fp);
     fwrite(&cluster_b21, sizeof(cluster_b21), 1, fp);
     fwrite(&cluster_b22, sizeof(cluster_b22), 1, fp);
     fwrite(&cluster_b23, sizeof(cluster_b23), 1, fp);
     fwrite(&cluster_b24, sizeof(cluster_b24), 1, fp);
     //3x volne misto
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     //prazdny adresar
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     //prvni cast msg.txt
     fwrite(&cluster_c1, sizeof(cluster_c1), 1, fp);
     //sem je jedno co zapisu, sou to vadne sektory - tedy realne byto meli byt stringy FFFFFFcosi cosi cosiFFFFFF
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
     //druha cast msg.txt
     fwrite(&cluster_c2, sizeof(cluster_c2), 1, fp);
     //zbytek disku budou 0
    for (long i=34; i < br.usable_cluster_count; i++)
    {
            fwrite(&cluster_empty, sizeof(cluster_empty), 1, fp);
    }
    fclose(fp);



    return 0;
}
 */
#endif //ZOS_ZMK_H
