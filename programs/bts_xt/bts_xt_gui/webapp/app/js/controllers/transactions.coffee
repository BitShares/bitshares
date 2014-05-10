angular.module("app").controller "TransactionsController", ($scope, $location, RpcService) ->
  $scope.transactions = []

  fromat_address = (addr) ->
    return "-" if !addr or addr.length == 0
    res = ""
    angular.forEach addr, (a) ->
      res += ", " if res.length > 0
      res += a[1]
    res
  format_amount = (delta_balance) ->
    return "-" if !delta_balance or delta_balance.length == 0
    first_asset = delta_balance[0]
    return "-" if !first_asset or first_asset.length < 2
    first_asset[1]

  $scope.load_transactions = ->
    RpcService.request("rescan").then (response) ->
      RpcService.request("get_transaction_history").then (response) ->
        #console.log "--- transactions = ", response
        $scope.transactions.splice(0, $scope.transactions.length)
        angular.forEach response.result, (val) ->
          $scope.transactions.push
            block_num: val.block_num
            trx_num: val.trx_num
            time: val.confirm_time
            amount: format_amount(val.delta_balance)
            from: fromat_address(val.from)
            to: fromat_address(val.to)
            memo: val.memo

  $scope.load_transactions()

  $scope.rescan = ->
    $scope.load_transactions()


#num: 647
#confirm_time: "20140506T050930"
#delta_balance: Array[1]
#fees: Array[1]
#0: Array[2]
#0: 0
#1: 167
#length: 2
#__proto__: Array[0]
#length: 1
#__proto__: Array[0]
#from: Array[1]
#0: Array[2]
#0: "PvyoCnxs13F2WyPhaoHgnjLf5S5Ck6GVf"
#1: "lllll123"
#length: 2
#__proto__: Array[0]
#length: 1
#__proto__: Array[0]
#memo: ""
#to: Array[1]
#0: Array[2]
#0: "6QiKWRBaQ8FQgFTVUxuEZ53iHVLeoWCX"
#1: "6QiKWRBaQ8FQgFTVUxuEZ53iHVLeoWCX"
#length: 2
#__proto__: Array[0]
#length: 1
#__proto__: Array[0]
#trx: Object
#trx_num: 0
#valid: true
#__proto__: Object
#1: Object
#length: 2
#__proto__: Array[0]
#__proto__: Object