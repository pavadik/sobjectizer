require 'mxx_ru/binary_unittest'

path = 'test/so_5/mchain/params_clone'

MxxRu::setup_target(
	MxxRu::BinaryUnittestTarget.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
