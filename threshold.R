library("xtable")
library("reshape2")

RESOURCES_DIR = "/Volumes/barracuda/Datasets/threshold"
RESULT_DIR = "/Users/esiragusa/Documents/Dissertation/tables"

LENGTH=100
ERRORS=seq(8,10)
METHODS = c(lemma="LEMMA",
            exact="DP",
            ilp="ILP",
            apx="APX")

options(scipen=999)


secsToTime <- function(secs)
{
  if (is.na(secs))
    return("--")
  
  ifelse (secs<3600,yes=sprintf("%d:%02d",floor(secs/60),floor(secs)%%60),no=sprintf("%01d:%02d:%02d",floor(secs/3600),(floor(secs/60))%%60,floor(secs)%%60))
}

secsToMinSec <- function(secs)
{
  if (is.na(secs))
    return("--")
  
  x = sprintf("%d:%02d",(floor(secs/60)),floor(secs)%%60)
  y = ""
  while (nchar(x) + nchar(y) < 7)
    y = paste(y, "0", sep="")
  paste("\\phantom{", y, "}", x, sep="")
}

TSV_FILE = paste(RESOURCES_DIR, "/threshold.tsv", sep='')
TEX_FILE = paste(RESULT_DIR, "/threshold.tex", sep='')

TABLE = read.csv(TSV_FILE, header=TRUE, sep='\t', colClasses=c("numeric", "numeric", "character", "character", "numeric", "numeric"))

table = subset(TABLE, length==LENGTH, select=c(shape, errors, method, threshold))
table = dcast(table, shape+errors~method, fill=0, value.var="threshold")
table$span = nchar(table$shape)
table$weigth = nchar(gsub("0","",table$shape))
table$exact = table$ilp
table = subset(table, select=c('shape','span','weigth','errors','lemma','exact','apx'))

times = subset(TABLE, length==LENGTH, select=c(shape, errors, method, seconds))
times = dcast(times, shape+errors~method, fill=0, value.var="seconds")
times = subset(times, select=c('ilp','exact'))
colnames(times) <- c('ilp'="ilp",'exact'="dp")
table$ilp <- times$ilp
table$dp <- times$dp

table = transform(table, errors=as.integer(errors), span=as.integer(span), weigth=as.integer(weigth), lemma=as.integer(lemma), exact=as.integer(exact), apx=as.integer(apx))
table = table[with(table, order(span,weigth,errors)), ]
colnames(table) <- c('shape'="$Q$-gram",'span'="$s(Q)$",'weigth'="$w(Q)$",'errors'='$k$','lemma'="LEMMA",'exact'="EXACT",'apx'="APX",'ilp'="ILP",'dp'="DP")

superheader = "\\multicolumn{4}{c}{ Instance } & \\multicolumn{3}{c}{ Threshold } & \\multicolumn{2}{c}{ Time [s] }\\\\\n"
superruler = "\\cmidrule{1-4} \\cmidrule{5-7} \\cmidrule{8-9}\n"
toprule=paste('\\toprule\n', superheader, superruler)

table <- xtable(table, align="crrrrrrrrr")
print(table, 
      file=TEX_FILE,
      floating=FALSE, 
      hline.after=NULL, 
      include.rownames = FALSE,
      sanitize.colnames.function=identity,
      add.to.col=list(pos=list(4, 7), command=c(" & ", " & ")),
      add.to.row=list(pos=list(-1, 0, nrow(table)), command=c(toprule, '\\midrule\n', '\\bottomrule\n')))
