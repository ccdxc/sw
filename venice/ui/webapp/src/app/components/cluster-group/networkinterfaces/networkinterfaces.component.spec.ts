/**-----
 Angular imports
 ------------------*/
 import { HttpClientTestingModule } from '@angular/common/http/testing';
import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { MatIconRegistry } from '@angular/material';
import { NoopAnimationsModule } from '@angular/platform-browser/animations';
import { RouterTestingModule } from '@angular/router/testing';
import { SharedModule } from '@app/components/shared/shared.module';
import { AuthService } from '@app/services/auth.service';
/**-----
 Venice web-app imports
 ------------------*/
import { ControllerService } from '@app/services/controller.service';
import { EventsService } from '@app/services/events.service';
import { BrowserService } from '@app/services/generated/browser.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { NetworkService } from '@app/services/generated/network.service';
import { SearchService } from '@app/services/generated/search.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { MessageService } from '@app/services/message.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LicenseService } from '@app/services/license.service';
import { StagingService } from '@app/services/generated/staging.service';
import { MaterialdesignModule } from '@lib/materialdesign.module';
import { PrimengModule } from '@lib/primeng.module';
import { ConfirmationService } from 'primeng/primeng';
import { WidgetsModule } from 'web-app-framework';
import { NetworkinterfacesComponent } from './networkinterfaces.component';




describe('NetworkinterfacesComponent', () => {
  let component: NetworkinterfacesComponent;
  let fixture: ComponentFixture<NetworkinterfacesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ NetworkinterfacesComponent ],
      imports: [
        FormsModule,
        ReactiveFormsModule,
        NoopAnimationsModule,
        SharedModule,
        HttpClientTestingModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule,
        WidgetsModule
      ],
      providers: [
        ControllerService,
        AuthService,
        ConfirmationService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        UIConfigsService,
        LicenseService,
        StagingService,
        AuthService,
        EventsService,
        SearchService,
        MonitoringService,
        MessageService,
        NetworkService,
        ClusterService,
        BrowserService
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NetworkinterfacesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
