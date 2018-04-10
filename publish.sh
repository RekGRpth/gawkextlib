# Publish the webpages of a given extension
#
# ./publish.sh  extension

# Create sftp script from webTOC
gawk -f _web/make_sftp.awk $1/webTOC > _web/_publish

# Run the sftp script
read -p "Username: " user
sftp $user@web.sourceforge.net < _web/_publish
