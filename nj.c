#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef char C;typedef long I;typedef void V;
typedef struct a{I t,r,d[3],p[2];}*A;
#define P printf
#define R return
#define V1(f) A f(A w)
#define V2(f) A f(A a,A w)
#define DO(n,x) {I i=0,_n=(n);for(;i<_n;++i){x;}}
I *ma(I n){R(I*)malloc(n*4);} V mv(I *d,I *s,I n){DO(n,d[i]=s[i]);}
I tr(I r,I *d){I z=1;DO(r,z=z*d[i]);R z;}
A ga(I t,I r,I *d){A z=(A)ma(5+tr(r,d));z->t=t,z->r=r,mv(z->d,d,r);
 R z;}
V1(iota){I n=*w->p;A z=ga(0,1,&n);DO(n,z->p[i]=i);R z;}
V2(plus){I r=w->r,*d=w->d,n=tr(r,d);A z=ga(0,r,d);
 DO(n,z->p[i]=a->p[i]+w->p[i]);R z;}
V2(from){I r=w->r-1,*d=w->d+1,n=tr(r,d);
 A z=ga(w->t,r,d);mv(z->p,w->p+(n**a->p),n);R z;}
V1(box){A z=ga(1,0,0);*z->p=(I)w;R z;}
V2(cat){I an=tr(a->r,a->d),wn=tr(w->r,w->d),n=an+wn;
 A z=ga(w->t,1,&n);mv(z->p,a->p,an);mv(z->p+an,w->p,wn);R z;}
V2(find){R 0;}
V2(rsh){I r=a->r?*a->d:1,n=tr(r,a->p),wn=tr(w->r,w->d);
 A z=ga(w->t,r,a->p);mv(z->p,w->p,wn=n>wn?wn:n);
 if(n-=wn)mv(z->p+wn,z->p,n);R z;}
V1(sha){A z=ga(0,1,&w->r);mv(z->p,w->d,w->r);R z;}
V1(id){R w;}V1(size){A z=ga(0,0,0);*z->p=w->r?*w->d:1;R z;}
V pi(I i){P("%ld ",i);}V nl(){P("\n");}
V pr(A w){I r=w->r,*d=w->d,n=tr(r,d);DO(r,pi(d[i]));nl();
 if(w->t)DO(n,P("< ");pr(w->p[i]))else DO(n,pi(w->p[i]));nl();}

C vt[]="+{~<#,";
A(*vd[])(A,A)={0,plus,from,find,0,rsh,cat},
 (*vm[])(A)={0,id,size,iota,box,sha,0};
I st[26]; I qp(I a){R  a>='a'&&a<='z';} I qv(I a){R a<'a';}
A ex(I *e){I a=*e;
 if(qp(a)){if(e[1]=='=')R st[a-'a']=ex(e+2);a= st[ a-'a'];}
 R qv(a)?(*vm[a])(ex(e+1)):e[1]?(*vd[e[1]])(a,ex(e+2)):(A)a;}
A noun(C c){A z;if(c<'0'||c>'9')R 0;z=ga(0,0,0);*z->p=c-'0';R z;}
I verb(C c){I i=0;for(;vt[i];)if(vt[i++]==c)R i;R 0;}
I *wd(C *s){I a,n=strlen(s),*e=ma(n+1);C c;
 DO(n,e[i]=(a=noun(c=s[i]))?a:(a=verb(c))?a:c);e[n]=0;R e;}

int main(){C s[99];I l;
 while(fgets(s,sizeof(s),stdin)){
    if ((l=strlen(s))<2) continue;s[l-1]=0;
    pr(ex(wd(s)));
 }}
