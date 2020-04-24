import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { configureTestSuite } from 'ng-bullet';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import { RouterTestingModule } from '@angular/router/testing';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { WidgetsModule } from 'web-app-framework';
import { SharedModule } from '@app/components/shared/shared.module';
import { ControllerService } from '@app/services/controller.service';
import { ConfirmationService } from 'primeng/primeng';
import { LogService } from '@app/services/logging/log.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MatIconRegistry } from '@angular/material';
import { SecurityService } from '@app/services/generated/security.service';
import { PrimengModule } from '@app/lib/primeng.module';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { MessageService } from '@app/services/message.service';
import { AuthService } from '@app/services/auth.service';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';

import { FirewallprofilesComponent } from './firewallprofiles.component';
import { NewfirewallprofileComponent } from './newfirewallprofile/newfirewallprofile.component';



describe('FirewallprofilesComponent', () => {
  let component: FirewallprofilesComponent;
  let fixture: ComponentFixture<FirewallprofilesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ FirewallprofilesComponent, NewfirewallprofileComponent ],
      imports: [
        RouterTestingModule,
        NoopAnimationsModule,
        HttpClientTestingModule,
        WidgetsModule,
        SharedModule,
        PrimengModule,
        FormsModule,
        ReactiveFormsModule,
        MaterialdesignModule,
      ],
      providers: [
        ControllerService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        SecurityService,
        UIConfigsService,
        LicenseService,
        AuthService,
        MessageService,
        ClusterService
      ],
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(FirewallprofilesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
