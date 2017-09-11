import React from 'react';
import PropTypes from 'prop-types';

import { flatbuffers } from './flatbuffers';
import { Strawpoll } from './strawpoll_generated.js';

import './App.css';

function printWrap(val) {
  console.log("val: ", val);
  return val;
}

function uOr(val, fn) {
  return val === undefined
    ? undefined
    : fn(val);
}

function ResultDescription(props) {
  return (
    <div style={{ marginBottom: 4, overflow: 'hidden' }}>
      <div style={{ float: 'left' }}>{props.text}</div>
      <div style={{ float: 'right', fontSize: '0.8em', paddingRight: '0.5em'}}>
        {props.percent.toFixed(2)}%
        ({props.votes} votes)
      </div>
    </div>
  );
}

ResultDescription.propTypes = {
  text: PropTypes.string.isRequired,
  percent: PropTypes.number.isRequired,
  votes: PropTypes.number.isRequired
};

const barColorsA = [ // eslint-disable-line no-unused-vars
  '#4b3947',
  '#e3c17d',
  '#f9f2e5',
  '#9e8e85'
];

const barColorsB = [ // eslint-disable-line no-unused-vars
  '#9eb4db',
  '#756a92',
  '#494165',
  '#2e205c',
  '#22144f'
];

const barColorsC = [ // eslint-disable-line no-unused-vars
  '#ffd0b0',
  '#ffa996',
  '#e6857c',
  '#d1626d',
  '#b54b63'
];

function ResultBar(props) {
  return (
    <div className={'ResultBar'}>
      <div style={{
        width: `${props.width.toFixed(2)}%`,
        height: '100%',
        backgroundColor: props.barColor
      }} />
    </div>
  );
}

ResultBar.propTypes = {
  width: PropTypes.number.isRequired,
  barColor: PropTypes.string.isRequired
};

function BarChart(props) {
  // FIXME talk about shalow copy and sorting as weird for a c++ user
  const options = props.options.slice().sort((a, b) => (a.votes < b.votes));

  const total = options.reduce((sum, an) => (sum + an.votes), 0);
  return (
    <div className={'BarChart'}>
      {options.map((an, i) => {
        const width = (an.votes / total) * 100;
        return (
          <div key={an.text} style={{ paddingBottom: '1em' }}>
            <ResultDescription
              text={an.text}
              percent={width}
              votes={an.votes}
            />
            <ResultBar
              width={width}
              barColor={props.barColors[i % props.barColors.length]}
            />
          </div>
        );
      })}
      <span style={{ fontSize: '0.9em' }}>Total Votes: {total}</span>
    </div>
  );
}

BarChart.propTypes = {
  options: PropTypes.arrayOf(PropTypes.shape({
    text: PropTypes.string.isRequired,
    votes: PropTypes.number.isRequired
  })).isRequired,
  barColors: PropTypes.arrayOf(PropTypes.string).isRequired
};

BarChart.defaultProps = {
  barColors: barColorsB
};

function Vote(props) {
  return (
    <ul className="Vote">
      {props.options.map((option, i) => (
        <li key={option.text}>
          <button onClick={() => { props.handleVote(i) }}>
            {option.text}
          </button>
        </li>
      ))}
    </ul>
  );
}

Vote.propTypes = {
  options: PropTypes.arrayOf(PropTypes.shape({
    text: PropTypes.string.isRequired
  })).isRequired,
  handleVote: PropTypes.func.isRequired
};

class StrawPoll extends React.Component {
  constructor(props) {
    super(props);

    this.state = {
      hasVoted: false,
      title: '',
      options: []
    };
  }

  componentDidMount() {
    this.setupWebSocketCommunication();
  }

  shouldComponentUpdate(nextProps, nextState) {
    return nextState.options.length > 0
      && nextState.options[0].text !== undefined;
  }

  setupWebSocketCommunication() {
    this.socket = new WebSocket(this.props.apiUrl);
    this.socket.binaryType = 'arraybuffer';

    this.socket.addEventListener('open', this.fetchPoll);
    this.socket.addEventListener('message', this.handleServerResponse);
    this.socket.addEventListener('close', this.handleDisconnect);
  }

  fetchPoll = (event) => {
    const builder = new flatbuffers.Builder(1024);
    Strawpoll.Request.startRequest(builder);
    Strawpoll.Request.addType(builder, Strawpoll.RequestType.Poll);
    builder.finish(Strawpoll.Request.endRequest(builder));

    this.socket.send(builder.asUint8Array());
  }

  fetchResult = (id) => {
    const builder = new flatbuffers.Builder(1024);
    Strawpoll.Request.startRequest(builder);
    Strawpoll.Request.addType(builder, Strawpoll.RequestType.Result);
    Strawpoll.Request.addVote(builder, builder.createLong(id));
    builder.finish(Strawpoll.Request.endRequest(builder));

    this.socket.send(builder.asUint8Array());
  }

  handleServerResponse = (event) => {
    //console.log("Server sent: ", event);

    const buf = new flatbuffers.ByteBuffer(new Uint8Array(event.data));
    const response = Strawpoll.Response.getRootAsResponse(buf);
    //console.log("Response Type: ", response.type());

    switch(response.type()) {
      case Strawpoll.ResponseType.Poll:
        this.updatePoll(response.poll());
        break;
      case Strawpoll.ResponseType.Result:
        this.updateResult(response.result());
        break;
      case Strawpoll.ResponseType.Error:
        console.error("Error: ", response.error());
        break;
      default:
        console.error("Invalid response type: ", response.type());
    };
  }

  updatePoll = (poll) => {
    console.log("updatePoll called ", this.state);
    this.setState((prevState) => ({
      title: poll.title(),
      options: Array.apply(null, { length: poll.optionsLength() })
      .map((v, i) => ({
        text: poll.options(i),
        votes: uOr(prevState.options[i], (op) => (op.votes))
      }))
    }));
  }

  updateResult = (result) => {
    console.log("updateResult called ", this.state);
    this.setState((prevState) => ({
      hasVoted: true,
      options: Array.apply(null, { length: result.votesLength() })
      .map((v, i) => ({
        text: uOr(prevState.options[i], (op) => (op.text)),
        votes: result.votes(i).toFloat64()
      }))
    }));
  }

  handleDisconnect = (event) => {
    console.error("WebSocket closed");
  }

  handleVote = (id) => {
    this.fetchResult(id);
  }

  render() {
    return (
      <div>
        <h1>{this.state.title}</h1>
        {this.state.hasVoted
          ? <BarChart options={this.state.options} barColors={barColorsC} />
          : <Vote options={this.state.options} handleVote={this.handleVote}/>
        }
      </div>
    );
  }
};

StrawPoll.propTypes = {
  apiUrl: PropTypes.string.isRequired
};

class App extends React.Component {
  render() {
    return (
      <div className="App">
        <StrawPoll apiUrl={'ws://localhost:3003'} />
      </div>
    );
  }
};

export default App;
