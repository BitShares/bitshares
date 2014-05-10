servicesModule = angular.module("app.services", [])
servicesModule.factory "ErrorService", (InfoBarService) ->
  errorMessage: null
  setError: (msg) ->
    @errorMessage = msg
    InfoBarService.message = null
  clear: ->
    @errorMessage = null

servicesModule.config ($httpProvider) ->
  $httpProvider.responseInterceptors.push "errorHttpInterceptor"
  return

# register the interceptor as a service, intercepts ALL angular ajax http calls
servicesModule.factory "errorHttpInterceptor", ($q, $location, ErrorService, $rootScope) ->
  dont_report_methods = ["open_wallet", "walletpassphrase"]
  success = (response) -> response
  error = (response) ->
    dont_report = false
    method = null
    error_msg = if response.data?.error?.message? then response.data.error.message else response.data
    if response.config? and response.config.url.match(/\/rpc$/)
      if error_msg.match(/check_wallet_unlocked/)
        dont_report = true
        $rootScope.$broadcast "event:walletUnlockRequired"
      if error_msg.match(/check_wallet_is_open/)
        dont_report = true
        $rootScope.$broadcast "event:walletOpenRequired"
      method = response.config.data?.method
      title = if method then "RPC error calling #{method}" else "RPC error"
      error_msg = "#{title}: #{error_msg}"
    else
      error_msg = "HTTP Error: " + error_msg
    console.log "#{error_msg} (#{response.status})", response
    method_in_dont_report_list = method and (dont_report_methods.filter (x) -> x == method).length > 0
    dont_report = dont_report or method_in_dont_report_list
    ErrorService.setError "#{error_msg} (#{response.status})" unless dont_report
#    if response.status is 401
#      $rootScope.$broadcast "event:loginRequired"
#    else if response.status >= 400 and response.status < 500
#      ErrorService.setError "RPC Error: #{msg} (#{response.status})"
#    else if response.status == 500
#      ErrorService.setError "Internal Server Error (#{response.status})"
    $q.reject response

  (promise) -> promise.then(success,error)

