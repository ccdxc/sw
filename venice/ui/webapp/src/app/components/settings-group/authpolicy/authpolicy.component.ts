import { Component, OnInit, ViewEncapsulation } from '@angular/core';
import { ControllerService } from '@app/services/controller.service';
import { AuthService } from '@app/services/generated/auth.service';
import { AuthAuthenticationPolicy, ApiStatus, AuthLdap, IAuthAuthenticationPolicy, AuthAuthenticators_authenticator_order, IApiStatus } from '@sdk/v1/models/generated/auth';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { FormControl } from '@angular/forms';

@Component({
  selector: 'app-authpolicy',
  templateUrl: './authpolicy.component.html',
  styleUrls: ['./authpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class AuthpolicyComponent implements OnInit {
  secretFormControl: FormControl = new FormControl('', []);
  enableUpdateSecretButton: boolean = false;
  authOrder = ['LOCAL', 'LDAP', 'RADIUS'];
  authPolicy: AuthAuthenticationPolicy = new AuthAuthenticationPolicy({ spec: { authenticators: { 'authenticator-order': ['LOCAL', 'LDAP'] } } });


  constructor(protected _controllerService: ControllerService,
    protected _authService: AuthService) { }

  ngOnInit() {
    const data: IAuthAuthenticationPolicy = {
      spec: {
        authenticators: {
          'authenticator-order': [AuthAuthenticators_authenticator_order.LOCAL, AuthAuthenticators_authenticator_order.LDAP, AuthAuthenticators_authenticator_order.RADIUS],
          ldap: {
            enabled: true,
            'base-dn': 'basedn',
            'bind-dn': 'binddn',
            'bind-password': 'bindpass',
            'attribute-mapping': {
              'email': 'email',
              'fullname': 'fullname',
              'user': 'user',
              'user-object-class': 'user-obj',
              'group-object-class': 'group-obj-class',
              'group': 'group',
              'tenant': 'tenant'
            },
            servers: [
              {
                'url': '10.1.1.10:8000',
                'tls-options': {
                  'server-name': 'server1',
                  'skip-server-cert-verification': false,
                  'start-tls': true,
                  'trusted-certs': 'example cert'
                }
              },
              {
                'url': '10.1.1.11:8000',
                'tls-options': {
                  'server-name': 'server1',
                  'skip-server-cert-verification': true,
                  'start-tls': false,
                  'trusted-certs': 'example cert'
                }
              }
            ]
          },
          local: {
            enabled: true
          },
          radius: {
            enabled: true,
            'nas-id': 'testid',
            servers: [
              {
                'url': '10.1.1.11:8000',
              },
              {
                'url': '10.1.1.11:8001',
              },
              {
                'url': '10.1.1.11:8002',
              }
            ]
          }

        }
      }
    };
    this.authPolicy = new AuthAuthenticationPolicy(data);


    // Setting the toolbar
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Settings', url: '' }, { label: 'Auth Policy', url: '' }]
    });
    this.refresh();
    console.log(this.authPolicy);
  }

  refresh() {
    this._authService.GetAuthenticationPolicy().subscribe(
      response => {
        const body = response.body as AuthAuthenticationPolicy;
        console.log(body);
      },
      error => {
        // TODO: Error handling
        if (error.body instanceof Error) {
          console.log('Auth service returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
        } else {
          console.log('Auth service returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
        }
      }
    );
  }

  swapRanks(newRank, oldRank) {
    // Since ranks should only ever be moving one up or one down
    // We swap the items
    const authOrder = this.authPolicy.spec.authenticators['authenticator-order'];
    const copy = Utility.getLodash().cloneDeep(authOrder);
    const temp = copy[newRank];
    copy[newRank] = copy[oldRank];
    copy[oldRank] = temp;
    this.authPolicy.spec.authenticators['authenticator-order'] = copy;
  }
}
