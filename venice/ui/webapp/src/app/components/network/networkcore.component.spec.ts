import { Component } from '@angular/core';
import { ComponentFixture, TestBed, async } from '@angular/core/testing';
import { MatIconRegistry } from '@angular/material';
import { RouterTestingModule } from '@angular/router/testing';
import { ControllerService } from '@app/services/controller.service';
import { NetworkcoreComponent } from './networkcore.component';



@Component({
  template: ''
})
class DummyComponent { }
describe('NetworkcoreComponent', () => {
  let component: NetworkcoreComponent;
  let fixture: ComponentFixture<NetworkcoreComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NetworkcoreComponent, DummyComponent],
      imports: [
        RouterTestingModule.withRoutes([
          { path: 'login', component: DummyComponent }
        ])
      ],
      providers: [
        ControllerService,
        MatIconRegistry,
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NetworkcoreComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
