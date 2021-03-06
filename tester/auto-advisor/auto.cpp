#include<bits/stdc++.h>
using namespace std ;
#define MAXN 101

// 全用[0]来存长度
char name[MAXN][MAXN] ;
//	  id    id_string
int credit[MAXN] ;
int score[MAXN] ;
int tried[MAXN] ;
char pre[MAXN][MAXN][MAXN][MAXN] ;
//       id    set   id   id_string

int sum_score = 0;
int sum_credit_tried = 0;
int sum_credit_get = 0;
int gratuate_credit = 0 ;

int main() {
	char tmp ;
	int cnt = 1 ;
	scanf("%c", &tmp) ;
	while(1) {
		//printf("TMP = %c\n", tmp) ;
		if ( tmp == '\n' || tmp == '\r')
			break ;

		name[cnt][0] = 0 ;
		while ( tmp != '|' ) {
			name[cnt][0] = name[cnt][0] + 1 ;
			name[cnt][ name[cnt][0] ] = tmp ;
			scanf("%c", &tmp) ;
		}

		// cout << name[cnt] << endl ;

		scanf("%d", &credit[cnt]) ;
		// printf("credit = %d\n", credit[cnt]) ;

		scanf("%c", &tmp) ;
		scanf("%c", &tmp) ;

		while ( tmp != '|') {
			char cur_pre_set[MAXN][MAXN] ;
			//		 id	id_string
			cur_pre_set[0][0] = 0 ;

			while ( tmp != ';' && tmp != '|') {
				char cur_pre[MAXN] ;
				cur_pre[0] = 0 ;

				while ( tmp != ',' && tmp != ';' && tmp != '|') {
					cur_pre[0] = cur_pre[0] + 1 ;
					cur_pre[cur_pre[0]] = tmp ;
					scanf("%c", &tmp) ;
					// puts("doing ','") ;
				}
				cur_pre_set[0][0] = cur_pre_set[0][0] + 1 ;
				cur_pre_set[ cur_pre_set[0][0] ][0] = cur_pre[0] ;
				for (int i = 1; i <= cur_pre[0]; ++i)
					cur_pre_set[ cur_pre_set[0][0] ][i] = cur_pre[i] ;
				if (tmp == ',')
					scanf("%c", &tmp) ;
				// puts("doing ';'") ;
			}
			pre[cnt][0][0][0] = pre[cnt][0][0][0] + 1 ;
			pre[cnt][ pre[cnt][0][0][0] ][0][0] = cur_pre_set[0][0] ;

			for (int i = 1; i <= cur_pre_set[0][0]; ++i ) {
				pre[cnt][ pre[cnt][0][0][0] ][i][0] = cur_pre_set[i][0] ;
				for (int j = 1; j <= cur_pre_set[i][0]; ++j ) {
					pre[cnt][ pre[cnt][0][0][0] ][i][j] = cur_pre_set[i][j] ;
				}
			}
			// pre[cnt].push_back(cur_pre_set) ;
			//
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
					scanf("%c", &tmp) ;
					break ;
				case'B' :
					score[cnt] = 3 ;
					scanf("%c", &tmp) ;
					break ;
				case'C' :
					score[cnt] = 2 ;
					scanf("%c", &tmp) ;
					break ;
				case'D' :
					score[cnt] = 1 ;
					scanf("%c", &tmp) ;
					break ;
				case'F' :
					score[cnt] = 0 ;
					scanf("%c", &tmp) ;
					break ;
				default:
					tried[cnt] = 0 ;
					score[cnt] = 0 ;
			}
//		}

		gratuate_credit += credit[cnt] ;
		if (tried[cnt]) {
			sum_credit_tried += credit[cnt] ;
			sum_score += score[cnt] * credit[cnt];
			if (score[cnt] > 0) {
				sum_credit_get += credit[cnt] ;
			}
		}
		
		scanf("%c", &tmp) ;
		++cnt ;

	}

	cnt = cnt - 1 ;

		/*
	for (int i = 1; i <= cnt; ++i ) {
		cout << name[i] << endl ;
		cout << credit[i] << endl ;

		cout << "pre:\n" ;
		for ( int j = 0; j < pre[i].size(); ++j ) {
			printf("\n\tset:\n") ;
			for ( int k = 0; k < pre[i][j].size(); ++k ) {
				cout << "\t\t" << pre[i][j][k] << endl ;
			}
		}
		cout << "end\n" ;
	}
		*/

	if ( sum_credit_tried == 0 )
		printf("GPA: 0.0\n") ;
	else {
		//printf("GPA: %.5f\n", 1.0 * sum_score / sum_credit_tried) ;
		int a, b ;
		a = sum_score ;
		b = sum_credit_tried ;
		int a1 = sum_score / sum_credit_tried ;
		int a2 = (sum_score * 10 / sum_credit_tried) % 10 ;
		int a3 = (sum_score * 100 / sum_credit_tried) % 10 ;
		if ( a3 >= 5 )
			++a2 ;
		if ( a2 ==10 ) {
			a2 = 0 ;
			a1 = a1 + 1 ;
		}
		printf("GPA: %d.%d\n", a1, a2) ;
	}
	printf("Hours Attempted: %d\n", sum_credit_tried) ;
	printf("Hours Completed: %d\n", sum_credit_get) ;
	printf("Credits Remaining: %d\n\n", gratuate_credit - sum_credit_get) ;


	printf("Possible Courses to Take Next\n") ;
	if (gratuate_credit == sum_credit_get ) {
		printf("  None - Congratulations!\n") ;
		return 0 ;
	} else {
		for ( int i = 1; i <= cnt; ++i ) {
			if (!score[i]) {
				// cout << "checking.. " << &name[i][1] << endl ;

				int tag = 0 ;
				if (pre[i][0][0][0] == 0)
					tag = 1 ;
				for ( int j = 1; j <= pre[i][0][0][0]; ++j ) {
					// cout << "n = " << int(pre[i][j][0][0]) << endl ;
					tag = 1 ;
					for ( int k = 1; k <= pre[i][j][0][0]; ++k ) {
						char cur[MAXN] ;
						cur[0] = pre[i][j][k][0] ;
						for (int ii = 1; ii <= pre[i][j][k][0]; ++ii )
							cur[ii] = pre[i][j][k][ii] ;
						// cout << '\t' << &cur[1] << endl ;
						int found = 0 ;
						for ( int l = 1; l <= cnt; ++l ) {
							int tag_cur = 1 ;
							if (name[l][0] != cur[0] ) {
								tag_cur = 0 ;
							} else {
								for (int ii = 1; ii <= cur[0]; ++ii ) {
									if ( name[l][ii] != cur[ii] ) {
										tag_cur = 0 ;
										break ;
									}
								}
							}
							if (tag_cur) {
								// cout << &(name[l][1]) << endl ;
								// cout << &(cur[1]) << endl<<endl ;
								//
								found = 1 ;
								if (score[l] == 0)
									tag = 0 ;
								break ;
							}
						}
						if (found == 0)
							tag = 0 ;
					}
					if ( tag == 1 )
						break ;
				}
				if ( tag == 1 ) {
					printf("  ") ;
					for ( int hhh = 1; hhh <= name[i][0]; ++hhh)
						printf("%c", name[i][hhh]) ;
					printf("\n") ;
					// cout << name[i] << endl ;
				}
				//getchar() ;
			}
		}
	}

	return 0 ;
}
