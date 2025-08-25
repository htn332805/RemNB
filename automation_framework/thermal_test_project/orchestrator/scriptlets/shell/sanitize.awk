# Use comma as both input and output field separators
BEGIN{FS=","; OFS=","}
# Pass header row unchanged
NR==1 {print; next}
{
  # Loop over all fields to trim spaces and convert N/A tokens to empty
  for(i=1;i<=NF;i++){
    gsub(/^[ \t]+|[ \t]+$/,"",$i)
    if($i=="N/A" || $i=="NA" || $i=="null") $i=""
  }
  # Print the sanitized row
  print
}
