require 'mxx_ru/binary_unittest'

path = 'test/so_5/environment/stop_guards/some_actions_after_stop'

MxxRu::setup_target(
	MxxRu::BinaryUnittestTarget.new(
		"#{path}/prj.ut.rb",
		"#{path}/prj.rb" )
)
