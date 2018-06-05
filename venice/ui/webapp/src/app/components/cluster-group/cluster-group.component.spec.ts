/**-----
 Angular imports
 ------------------*/
 import { async, ComponentFixture, TestBed } from '@angular/core/testing';
 import { RouterTestingModule } from '@angular/router/testing';
 import { HttpClientTestingModule } from '@angular/common/http/testing';
 import { Component } from '@angular/core';
 
/**-----
 Venice UI -  imports
 ------------------*/
import { ClusterGroupComponent } from './cluster-group.component';
import { ControllerService } from '@app/services/controller.service';

/**-----
 Third-parties imports
 ------------------*/
import { MatIconRegistry } from '@angular/material';

@Component({
  template: ''
})
class DummyComponent { }

describe('ClusterGroupComponent', () => {
  let component: ClusterGroupComponent;
  let fixture: ComponentFixture<ClusterGroupComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        ClusterGroupComponent,
        DummyComponent
      ],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ]),
        HttpClientTestingModule
      ],
      providers: [
        ControllerService,
        MatIconRegistry
      ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ClusterGroupComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
