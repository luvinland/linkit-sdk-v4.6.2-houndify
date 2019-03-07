use File::Copy;

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

my $new_bak_dir = "\.\.\/decoupling\/template\/";
if (opendir(DIR, $new_bak_dir))
{
	  while (my $file_name = readdir(DIR))
    {
    	  my $file_full_path = $new_bak_dir.$file_name;
    	  if (open(inputFile, "<$file_full_path"))
    	  {
    	  	  print $file_name."\n";
    	  	  close(inputFile);
            rename $new_bak_dir.$file_name, $new_bak_dir.$file_name."\.bak";
            move($new_bak_dir.$file_name."\.bak", "\.\.\/decoupling\/template\/bak\/");
        }
    }
}
