import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { SettingsComponent } from '@app/components/settings-group/settings/settings.component';


const routes: Routes = [
  {
    path: '',
    redirectTo: 'settings',
    pathMatch: 'full'
  },
  {
    path: 'authpolicy',
    children: [
      {
        path: '',
        loadChildren: '@app/components/settings-group/authpolicy/authpolicy.module#AuthpolicyModule'
      }
    ]
  },
  {
    path: 'users',
    children: [
      {
        path: '',
        loadChildren: '@app/components/settings-group/users/users.module#UsersModule'
      }
    ]
  },
  {
    path: 'settings',
    component: SettingsComponent,
  },
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class SettingsGroupRoutingModule { }
