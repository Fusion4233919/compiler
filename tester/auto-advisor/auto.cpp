#include<bits/stdc++.h>
using namespace std ;
#define MAXN 1001

string name[MAXN] ;
int credit[MAXN] ;
int score[MAXN] ;
int tried[MAXN] ;
vector<vector<string>> pre[MAXN] ;

int main() {
	char tmp ;
	int cnt = 1 ;
	scanf("%c", &tmp) ;
	while(1) {
		//printf("TMP = %c\n", tmp) ;
		if ( tmp == '\n' || tmp == '\r')
			break ;

		while ( tmp != '|' ) {
			name[cnt] = name[cnt] + tmp ;
			scanf("%c", &tmp) ;
		}

		// cout << name[cnt] << endl ;

		scanf("%d", &credit[cnt]) ;

		scanf("%c", &tmp) ;
		scanf("%c", &tmp) ;

		while ( tmp != '|') {
			vector<string> cur_pre_set ;
			while ( tmp != ';' && tmp != '|') {
				string cur_pre = "" ;
				while ( tmp != ',' && tmp != ';' && tmp != '|') {
					cur_pre = cur_pre + tmp ;
					scanf("%c", &tmp) ;
					// puts("doing ','") ;
				}
				cur_pre_set.push_back(cur_pre) ;
				if (tmp == ',')
					scanf("%c", &tmp) ;
				// puts("doing ';'") ;
			}
			pre[cnt].push_back(cur_pre_set) ;
			if (tmp == ';')
				scanf("%c", &tmp) ;
			// puts("doing '|'") ;
		}

		scanf("%c", &tmp) ;
		
		//printf("tmp = %c\n ", tmp ) ;

		tried[cnt] = 1 ;
//		if ( tmp == '\n' || tmp == '\r' ) {
//			tried[cnt] = 0 ;
//		} else {
			switch(tmp) {
				case 'A' : 
					score[cnt] = 4 ;
					break ;
				case'B' :
					score[cnt] = 3 ;
					break ;
				case'C' :
					score[cnt] = 2 ;
					break ;
				case'D' :
					score[cnt] = 1 ;
					break ;
				case'F' :
					score[cnt] = 0 ;
					break ;
				default:
					tried[cnt] = 0 ;
					++cnt ;
			}
//		}
		scanf("%c", &tmp) ;

	}

	for (int i = 1; i <= cnt; ++i ) {
		cout << name[i] << endl << "pre:\n" ;

		for ( int j = 0; j < pre[i].size(); ++j ) {
			printf("\n\tset:\n") ;
			for ( int k = 0; k < pre[i][j].size(); ++k ) {
				cout << "\t\t" << pre[i][j][k] << endl ;
			}
		}
		cout << "end\n" ;
	}


	return 0 ;
}
