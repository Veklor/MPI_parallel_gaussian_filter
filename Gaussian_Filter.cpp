#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
#include "mpi.h"    //MPI Rutinlerini Kullanabilmek Icin

using namespace std;

int main(int argc, char* argv[]) {

	// MPI işlemlerini kullanabilmek için gerekli değişkenler
	char processorName[BUFSIZ];
	int  nameLength,
		rank,
		size;

	double baslangicZamani,
		bitisZamani;

	//----------------------------------------DOSYA DEGİSKENLERİ-----------------------------------------------

	ofstream cikisDosyasi;
	ifstream girisDosyasi;

	string satir;

	string dosyaninAdresi = argv[1];
	girisDosyasi.open(dosyaninAdresi);// Dosyayı Oku


	//---------------------------------------------GENEL DEGİSKENLER--------------------------------

	int* girisDosyasiElemanlari; //Giriş dosyasının elemanlarını tutan dizi
	int* cikisDosyasiElemanlari; //Çıkış dosyasının elemanlarını tutan dizi
	int* herRankinGeciciDizisi;
	int* herRankinFiltreliDizisi;

	int satir_sayisi;
	int sutun_sayisi;
	int girisDiziBoyutu;
	int fark;

	int root;
	

	int herRankinGeciciDizisi_SatirSayisi;
	int herRankinGeciciDizisi_SutunSayisi;
	int herRankinGeciciDizisi_ToplamBoyut;

	int cevredekiDegerlerToplami = 0, cevredekiDegerlerinOrtalamasi = 0;

	//-----------------------------------------------------------MPI RUTİNLERİ----------------------------------

	MPI_Init(&argc, &argv);
	MPI_Status status, status1;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);



	//----------------------------------------------------RANK 0 -------------------------------------------------

	if (rank == 0) {
	
		
		girisDosyasi >> satir_sayisi >> sutun_sayisi;

		fark = satir_sayisi % size;

		girisDiziBoyutu = (satir_sayisi - fark) * sutun_sayisi;


		girisDosyasiElemanlari = new int[girisDiziBoyutu];
		cikisDosyasiElemanlari = new int[girisDiziBoyutu];


		//------Rank 0'ın kendine ait kısmına uygun diziler ve değişkenler

		herRankinGeciciDizisi_SatirSayisi = (satir_sayisi - fark) / size;
		herRankinGeciciDizisi_SutunSayisi = sutun_sayisi;

		//------------dosyanın içeriğini oku ve giris dizisine yaz--------------
		//printf("[INFO] Filtrelemeden onceki hali:\n");

		for (int i = 0; i < satir_sayisi - fark; i++)
		{
				/*
				if (i < 10)	//-----------Kontrol ettt-------
				{
					cout << endl;
				}//--------------------*/

			for (int j = 0; j < sutun_sayisi; j++)
			{
				girisDosyasi >> girisDosyasiElemanlari[i * sutun_sayisi + j];

				/*
				if (i < 10 && j < 10)//-------------kontrol------------
				{
					cout << girisDosyasiElemanlari[i * sutun_sayisi + j] << "\t";
				}//---------------------*/
			}
		}
		//cout << endl;

		root = 0;

		baslangicZamani = MPI_Wtime(); //--------------------ZAMANI BASLAT---------------

	}

	
	MPI_Bcast(&root, 1, MPI_INT, 0, MPI_COMM_WORLD);
	
	// ELEMANLAR BAŞARIYLA ALINDI ŞİMDİ İŞLEMEYE SIRA GELDİ...
	//-----------------------------Diğer rankları haberdar et---------------------
	MPI_Bcast(&herRankinGeciciDizisi_SatirSayisi, 1, MPI_INT, root, MPI_COMM_WORLD);
	MPI_Bcast(&herRankinGeciciDizisi_SutunSayisi, 1, MPI_INT, root, MPI_COMM_WORLD);

	// Gerekli dizileri olustur
	herRankinGeciciDizisi_ToplamBoyut = herRankinGeciciDizisi_SatirSayisi * herRankinGeciciDizisi_SutunSayisi;
	herRankinGeciciDizisi = new int[herRankinGeciciDizisi_ToplamBoyut];
	herRankinFiltreliDizisi = new int[herRankinGeciciDizisi_ToplamBoyut];

	// Herkese kısmını dagıt
	MPI_Scatter(girisDosyasiElemanlari, herRankinGeciciDizisi_ToplamBoyut, MPI_INT, herRankinGeciciDizisi, herRankinGeciciDizisi_ToplamBoyut, MPI_INT, root, MPI_COMM_WORLD);


	// ISLEMEYE BASLAAAAAA

	for (int i = 0; i < herRankinGeciciDizisi_SatirSayisi; i++)
	{
		for (int j = 0; j < herRankinGeciciDizisi_SutunSayisi; j++)
		{
			if (i<2 || j<2 || i>herRankinGeciciDizisi_SatirSayisi - 3 || j>herRankinGeciciDizisi_SutunSayisi - 3)
			{
				herRankinFiltreliDizisi[i * herRankinGeciciDizisi_SutunSayisi + j] = herRankinGeciciDizisi[i * herRankinGeciciDizisi_SutunSayisi + j];
			}
			else
			{
				for (int k = -2; k <= 2; k++)
				{
					for (int l = -2; l <= 2; l++)
					{
						cevredekiDegerlerToplami += herRankinGeciciDizisi[((i + k) * herRankinGeciciDizisi_SutunSayisi) + j + l];
					}
				}
				cevredekiDegerlerinOrtalamasi = cevredekiDegerlerToplami / 25;
				herRankinFiltreliDizisi[i * herRankinGeciciDizisi_SutunSayisi + j] = cevredekiDegerlerinOrtalamasi;
				cevredekiDegerlerToplami = 0;
			}
		}
	}

	//----------------------DİĞER KISIMLARI EKLE-----------------------
	MPI_Gather(herRankinFiltreliDizisi, herRankinGeciciDizisi_ToplamBoyut, MPI_INT, cikisDosyasiElemanlari, herRankinGeciciDizisi_ToplamBoyut, MPI_INT, root, MPI_COMM_WORLD);

	if (rank == root)
	{

		bitisZamani = MPI_Wtime();


		//------------Dosya adini düzenle-------------

		//printf("[INFO] Rank 0 dosyayi yaziyor...\n");
		dosyaninAdresi = dosyaninAdresi.substr(22, dosyaninAdresi.length() - 4);
		string cikis_dosyasi_ismi = dosyaninAdresi.append("_filteredWithCC.txt");
		cikisDosyasi.open(cikis_dosyasi_ismi.c_str()); // Çıkış dosyasını oluştur.
		//printf("[INFO] Rank 0 cikis dosyasini olusturdu.\n");
		//printf("[INFO] Filtrelemeden sonraki hali:\n");
		//-------------------------------DOSYAYA YAZ-------------------------------

		for (int i = 0; i < satir_sayisi - fark; i++)
		{
			/*
			if (i < 10) //---------kontrol ----
			{
				cout << endl;
			}//------------------------*/

			for (int j = 0; j < sutun_sayisi; j++)
			{
				cikisDosyasi << cikisDosyasiElemanlari[i * sutun_sayisi + j] << "\t";

				/*
				if (i < 10 && j < 10)//----------kontrol----
				{
					cout << cikisDosyasiElemanlari[i * sutun_sayisi + j] << "\t";
				}//---------------*/
			}
			cikisDosyasi << endl;
		}

		//cout << endl;
		
		printf("Paralellestirilmis toplam gecen sure: %.5f\n", (bitisZamani - baslangicZamani) * 1000);

		girisDosyasi.close();
		cikisDosyasi.close();

		delete[] girisDosyasiElemanlari;
		delete[] cikisDosyasiElemanlari;
	}



	delete[] herRankinGeciciDizisi;
	delete[] herRankinFiltreliDizisi;


	MPI_Finalize();


	return 0;
}


