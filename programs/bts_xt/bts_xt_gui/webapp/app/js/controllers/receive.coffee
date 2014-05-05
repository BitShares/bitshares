angular.module("app").controller "ReceiveController", ($scope, $location, RpcService) ->
  $scope.new_address_label = ""
  $scope.addresses = []

  refresh_addresses = ->
    RpcService.request('list_receive_addresses').then (response) ->
      $scope.addresses.splice(0, $scope.addresses.length)
      angular.forEach response.result, (val) ->
        $scope.addresses.push({label: val[1], address: val[0]})

  refresh_addresses()

  $scope.create_address = ->
    RpcService.request('getnewaddress', [$scope.new_address_label]).then (response) ->
      refresh_addresses()

  $scope.import_key = ->
    RpcService.request('import_private_key', [$scope.pk_value, $scope.pk_label]).then (response) ->
      $scope.pk_value = ""
      $scope.pk_label = ""
      refresh_addresses()