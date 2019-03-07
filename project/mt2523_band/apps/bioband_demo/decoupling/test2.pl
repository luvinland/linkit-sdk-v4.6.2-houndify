use File::Copy;


my $origin_directory = "\.\.\/decoupling\/template\/origin\/";
my $template_dir = "\.\.\/decoupling\/template\/";
my $bak_directory = "\.\.\/decoupling\/template\/bak\/";
if (opendir(DIR, $bak_directory))
{
    while (my $file_name = readdir(DIR))
    {
	      unlink "\.\.\/decoupling\/template\/bak\/".$file_name;
    }
}
else
{
	  mkdir $bak_directory;
}
if (opendir(DIR, $template_dir))
{
	  while (my $file_name = readdir(DIR))
    {
    	  my $file_full_path = $template_dir.$file_name;
        if (-f $file_full_path)
    	  {
    	      if (!-d $origin_directory)
            {
            	  mkdir $origin_directory;
            }
            move($file_full_path , $origin_directory);
        }
    }
}


open(inputFile, ">\.\.\/decoupling\/template\/text_1.txt");
print inputFile "this is file 1\n";
close(inputFile);
open(inputFile, ">\.\.\/decoupling\/template\/text_2.txt");
print inputFile "this is file 2\n";
close(inputFile);

if (opendir(DIR, $template_dir))
{
	  while (my $file_name = readdir(DIR))
    {
    	  my $file_full_path = $template_dir.$file_name;
    	  if (-f $file_full_path && -d $origin_directory)
    	  {
    	  	  rename $file_full_path, $file_full_path."\.bak";
            move($file_full_path."\.bak", $bak_directory);
        }
    }
}
if (opendir(DIR, $origin_directory))
{
	  while (my $file_name = readdir(DIR))
    {
    	  my $file_full_path = $origin_directory.$file_name;
    	  if (-f $file_full_path)
    	  {
            move($file_full_path, $template_dir);
        }
    }
}
rmdir $origin_directory;