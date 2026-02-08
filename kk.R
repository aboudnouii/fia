Qualitative variable: 
Example 01: 
gender=sample( c("Male","Female"),100,replace=TRUE) 
freq=table(gender)#calculate the frequency 
percent=prop.table(table(gender))# Calculate the percentage of each category 
par(mfrow=c(1,2)) 
barplot(freq,col=2,main="Barplot of Gender",ylab="frequency")#frequency barchart  
label=c("F","M") 
pie(freq,labels=paste(label,percent,"%"),col=c("red","blue"),main="pie chart of 
Gender",border="white") 
library(plotrix) 
pie3D(freq,labels=paste(label,percent,"%"),col=c("red","blue"),main="pie chart of 
Gender",border="white") 
Remark: par(mfrow=c(number of rows,number of columns)) : is a graphical 
parameter that allows to display several plots in the same window, arranged in a 
matrix of rows and columns. 
Quantitative variable (discret): 
Example 02:  
X=c(1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,4,4,5,5) 
Freq=table(X) 
cumFreq=cumsum(Freq)#cumulative frequency 
df=data.frame(Freq,cumFreq) 
1 
par(mfrow=c(1,2)) 
barplot(Freq,col=2,main="Barplot of children’s number",ylab="frequency") 
plot(df$X,df$cumFreq,main="Cumulative frequency curve", xlab="number of 
children",ylab="Cumulative frequency")# plot the cumulative frequency curve 
Q=quantile(X)#calculate quantiles(0%,25% ,50%,75%,100%) 
abline(v=Q[c(2,3,4)],lty=2)#To add Q1,Q2, and Q3 to the cumulative curve 
plot(ecdf(X))# plot the cumulative relative frequency curve (Ogive) 
summary(X)#it provide all measures of central tendancy except the mode. 
Mo= names(sort(table(X),decreasing=TRUE))[1] 
########dispersion parameters 
var(X)# variance 
sd(X)#standard deviation 
range(X)# give min and max value  
IQR(X) 
boxplot(X,main=”Boxplot of X”, yalab=”Values”,col=”blue) 
boxplot.stats(X) 
boxplot(X,main="Boxplot of X", yalab="Values",col="blue") 
abline(h=c(min(X),max(X),Q),lty=2) 
Quantitative variable (continuous): 
Example 03:  
data(mtcars) 
mtcars 
weight=mtcars$wt 
hist(weight,main="Histogram of weight",xlab="weight",col=4) 
boxplot(weight) 
boxplot.stats(weight) 
boxplot.stats(weight) $stat[2]-1.5*(IQR(weight)) 
2 
boxplot.stats(weight)[2]+(1.5*IQR(weight)) 
summary(weight) 
mo=names(sort(table(weight),decreasing=TRUE))[1] 
mo 
carcons=mtcars$mpg#cars consumption 
plot(weight,carcons)# scatterplot 
abline(lm(carcons ~ weight,data=mtcars),col=3) 
cor(carcons,weight)#linear correlation 
Spearman and Kendall tau 
Rank=c(9,7,6,5,4,3,2,1,8,6) 
X=c(2,4,6,8,10,12,14,16,5,9) 
cor(Rank,X,method="spearman") 
cor(Rank,X,method="kendall") 
Chi-square test 
data(HairEyeColor) 
HEC=HairEyeColor 
Hair_eye=margin.table(HEC,c(1,2)) 
chisq.test(Hair_eye) 
##################################################################” 
Missing Values: 
data=read.csv("missing values.csv",sep=";") 
is.na(data) 
colSums(is.na(data)) 
rowSums(is.na(data)) 
data$height[is.na(data$height)]=mean(data$height,na.rm=TRUE) 
data$weight[is.na(data$weight)]=median(data$weight,na.rm=TRUE) 
summary(data$height) 
cor(data$height,data$weight) 
