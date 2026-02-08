x=c(1,2,3,4)
y=c(5,6,7,8)
df=data.frame(x,y)
df$z=c(9,10,11,12)
df

gender=c("M","F","F","M","M","F","M","F","M","F","M","M","M","F","F")
grade=c(6,7,13,14,15,17,10,11,9,8,7,16,12,15,14)
std=c(2,3,8,9,10,12,6,5,4,3,2,11,7,13,12)
ks=data.frame(gender,grade,std)
ks
freq=table(df$gender)